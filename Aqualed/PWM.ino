/*
   AQUALED PWM LED support functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#include <Arduino.h>

#ifndef USE_ADAFRUIT_LIBRARY
  #include "PCA9685.h"
PCA9685 pwmController;
  #define PWM_I2C_SETVALUE(x, y) pwmController.setChannelPWM(x, y)
#else
  #include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm_i2c = Adafruit_PWMServoDriver();
  #define PWM_I2C_SETVALUE(x, y) pwm_i2c.setPWM(x, 0, y)
#endif


static void setupPWMPins ()
{

#ifndef USE_ADAFRUIT_LIBRARY
        pwmController.resetDevices(); // Software resets all PCA9685 devices on Wire line
        pwmController.init(B000000); // Address pins A5-A0 set to B000000
        pwmController.setPWMFrequency(PWM_I2C_FREQ); // Default is 200Hz, supports 24Hz to 1526Hz
#else
        pwm_i2c.begin();
        pwm_i2c.setPWMFreq(PWM_I2C_FREQ);
#endif
        getCurrentTime ();
        // setup pins
        for (byte i = 0; i < PWMS; i++)
        {
                initPWM ( i );
                memset (&pwmRuntime[i], 0, sizeof pwmRuntime[i]);
                updateChannelTimes (i);
                recoverSunsetAndSunrise (i);
        }
}

static void initPWM (byte i)
{
        if (pwmSettings[i].isI2C != 0) return;
        pinMode(pwmSettings[i].pin, OUTPUT);
        digitalWrite(pwmSettings[i].pin, OFF);
}

static void updateChannelTimes (byte i)
{
        pwmRuntime[i].startTime = (long)(pwmSettings[i].onHour) * 60 * 60 + (long)(pwmSettings[i].onMinute)* 60;
        pwmRuntime[i].stopTime = (long)(pwmSettings[i].offHour) * 60 * 60 + (long)(pwmSettings[i].offMinute)* 60;
        pwmRuntime[i].sunsetTime =  (long) pwmSettings[i].sunsetLenght * 60;
        pwmRuntime[i].sunriseTime =  (long)pwmSettings[i].sunriseLenght * 60;
}

static void recoverSunsetAndSunrise (byte i)
{
        if ( getSunsetSeconds (i)  == true)
        {
                pwmRuntime[i].recoverLastState = true;
                pwmRuntime[i].valueGoal = (
                        ((double)pwmRuntime[i].secondsLeft/(double)pwmRuntime[i].sunsetTime)*
                        (pwmSettings[i].valueDay - getNightValue(i)) +getNightValue(i));
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
        }
        else
        if ( getSunriseSeconds (i)  == true)
        {
                pwmRuntime[i].recoverLastState = true;
                pwmRuntime[i].valueGoal = ((1-(double)pwmRuntime[i].secondsLeft/(double)pwmRuntime[i].sunriseTime)*pwmSettings[i].valueDay);
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
        }
        else pwmRuntime[i].recoverLastState = false;
}

// sciemnianie/rozjasnianie
static void pwmStep (byte i)
{
        // dimming complete, do nothing
        if (pwmRuntime[i].valueCurrent == pwmRuntime[i].valueGoal)
        {
                goalReached (i);
                return;
        }

        if (pwmRuntime[i].valueGoal > pwmRuntime[i].valueCurrent)
        {
                pwmRuntime[i].valueCurrent = pwmRuntime[i].valueCurrent +  pwmRuntime[i].step;
                if (pwmRuntime[i].valueCurrent > pwmRuntime[i].valueGoal) pwmRuntime[i].valueCurrent=pwmRuntime[i].valueGoal;
        }
        else
        {
                pwmRuntime[i].valueCurrent = pwmRuntime[i].valueCurrent -  pwmRuntime[i].step;
                if (pwmRuntime[i].valueCurrent < pwmRuntime[i].valueGoal) pwmRuntime[i].valueCurrent=pwmRuntime[i].valueGoal;
        }
}

static void goalReached (byte i)
{
        pwmRuntime[i].dimmingStart = false;
        pwmRuntime[i].recoverLastState = false;
        pwmRuntime[i].step = 0;
        pwmRuntime[i].secondsLeft = 0;
        resetFlags (i);
}

static void resetFlags (byte i)
{
        pwmRuntime[i].isSunset = false;
        pwmRuntime[i].isSunrise = false;
        //pwmRuntime[i].recoverLastState = false;
}

static void forcePWMRecovery (boolean test)
{
        for (byte i = 0; i < PWMS; i++)
        {
                if (test && pwmRuntime[i].testMode!=true) continue;
                resetFlags (i);
                pwmRuntime[i].dimmingStart = false;
                recoverSunsetAndSunrise(i);
        }
}

static void forceDimmingRestart ()
{
        for (byte i = 0; i < PWMS; i++)
        {
                resetFlags (i);
                pwmRuntime[i].dimmingStart = false;
        }
}

static double getNightValue (byte i)
{
        double result = 0.0;
        if (pwmSettings[i].isNightLight)
        {
                if (pwmSettings[i].useLunarPhase==0) result = (double)pwmSettings[i].valueNight;
                else result = (double)pwmSettings[i].valueNight * (double)moonPhases[moonPhase] * 0.01;
        }
        return result;
}

static void initDimming (byte i, double dimmingScale, long dimmingTime)
{
        if (pwmRuntime[i].valueCurrent!=pwmRuntime[i].valueGoal && pwmRuntime[i].dimmingStart == false)
        {
                pwmRuntime[i].dimmingStart = true;
                pwmRuntime[i].step = (double) ( (double)  dimmingScale  / (double) ((dimmingTime * 1000) * PWM_RESOLUTION_R));
                if ( pwmRuntime[i].step < PWM_MIN_STEP) pwmRuntime[i].step = PWM_MIN_STEP;
        }
}

// calculate and set pwm value and drive led
static void pwm( byte i )
{
        bool state = getState (i);
        pwmRuntime[i].isNight= false;

        // readjust times - millis() is not accurate over longer periods
        if (pwmRuntime[i].ticks > PWM_ADJUST_STEP_TICKS)
        {
                resetFlags (i);
                pwmRuntime[i].ticks = 0;
        }

        //test mode
        if (lampOverheating == true || max_watts_exceeded == true)
        {
                pwmRuntime[i].valueCurrent = 0;
                pwmRuntime[i].dimmingStart = false;
        }
        else
        //test mode
        if (pwmRuntime[i].testMode)
        {
                //pwmRuntime[i].valueCurrent = pwmRuntime[i].valueTest;
                pwmRuntime[i].dimmingStart = false;
        }
        else
        // force off
        if (pwmSettings[i].enabled == 0 || settings.forceOFF == 1)
        {
                pwmRuntime[i].valueGoal = 0;
                initDimming (i, pwmRuntime[i].valueCurrent,settings.pwmDimmingTime);
        }
        else
        // force night
        if (settings.forceNight == 1)
        {
                pwmRuntime[i].valueGoal = 0;
                if (pwmSettings[i].isNightLight)
                        pwmRuntime[i].valueGoal = getNightValue(i);

                initDimming (i, abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueNight), settings.pwmDimmingTime);
                pwmRuntime[i].isNight= true;
        }
        else
        // ambient/user program
        if (settings.forceAmbient == 1 && pwmSettings[i].isProg &&  pwmSettings[i].isProg == 1)
        {
                pwmRuntime[i].valueGoal = pwmSettings[i].valueProg;
                initDimming (i, abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueProg), settings.pwmDimmingTime);
        }
        else
        if (pwmRuntime[i].recoverLastState)
        {
                // just dim
        }
        // night light
        else if (!state && pwmSettings[i].isNightLight == 1)
        {
                pwmRuntime[i].isNight= true;
                pwmRuntime[i].valueGoal = getNightValue(i);
                initDimming (i, abs(pwmRuntime[i].valueCurrent-  pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
        }
        else
        //sunset
        if ( getSunsetSeconds (i) == true)
        {
                pwmRuntime[i].isSunset = true;
                pwmRuntime[i].valueGoal = getNightValue(i);
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal),pwmRuntime[i].secondsLeft);
        }
        else
        //sunrise
        if ( getSunriseSeconds (i)  == true)
        {
                pwmRuntime[i].isSunrise = true;
                pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                initDimming (i,abs(pwmRuntime[i].valueCurrent-(double)pwmSettings[i].valueDay),pwmRuntime[i].secondsLeft);
        }
        // day
        else if (state)
        {
                pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueDay),settings.pwmDimmingTime);
        }
        // scheduled off
        else if (!state && pwmSettings[i].isNightLight == 0)
        {
                pwmRuntime[i].valueGoal = 0;
                initDimming (i, pwmRuntime[i].valueCurrent,settings.pwmDimmingTime);
        }

        pwmStep (i);
        pwmRuntime[i].ticks++;

        // no change
        if (pwmRuntime[i].pwmLast != pwmRuntime[i].valueCurrent)
        {
                if (pwmSettings[i].enabled && pwmSettings[i].watts && pwmRuntime[i].valueCurrent)
                {
                        pwmRuntime[i].watts=(double)(((double)pwmRuntime[i].valueCurrent / (double)PWM_I2C_MAX) * (double)pwmSettings[i].watts);
                }
                else pwmRuntime[i].watts=0;
        }

        int val = (int) pwmRuntime[i].valueCurrent;

        if (pwmSettings[i].isI2C == 0)
        {
                if (pwmSettings[i].invertPwm == 1)
                        val = mapRound(val,  PWM_I2C_MAX, PWM_I2C_MIN,  0, 255);
                else
                        val = mapRound(val, PWM_I2C_MIN, PWM_I2C_MAX,  0, 255);

                // logarithmic dimming table (gamma correction), experimental, works best if max 100%
                      #ifndef NO_DIMMING_TABLE
                if (dimming && settings.softDimming == 1 && (int) val != (int) pwmRuntime[i].valueGoal)
                        val = dimmingTable[val];
                      #endif
                analogWrite( pwmSettings[i].pin, val);
        }
        else
        {
                // pwm module doesn't need constant updates
                if (pwmRuntime[i].pwmLast != pwmRuntime[i].valueCurrent)
                {
                        if (pwmSettings[i].invertPwm == 1)
                                val = mapRound(val, PWM_I2C_MAX, 0.0, PWM_I2C_MIN, PWM_I2C_MAX);

                        PWM_I2C_SETVALUE (pwmSettings[i].pin, val);
                }
        }
        pwmRuntime[i].pwmLast = pwmRuntime[i].valueCurrent;
}

// check if scheduled on or off
static bool getState (byte i)
{
        if (pwmRuntime[i].stopTime < pwmRuntime[i].startTime)
        {
                if (currTime>=pwmRuntime[i].startTime && currTime < 86400) return true;
                else if (currTime>=0 && currTime <pwmRuntime[i].stopTime) return true;
        }
        else
        {
                if (currTime>=pwmRuntime[i].startTime && currTime < pwmRuntime[i].stopTime) return true;
        }
        return false;
}

// calculate remaining sunset time (if any)
static bool getSunsetSeconds (byte i)
{
        long s = 0;
        s = pwmRuntime[i].stopTime - currTime;
        if (s < 0) s+=86400;
        if (s < pwmRuntime[i].sunsetTime)
        {
                pwmRuntime[i].secondsLeft  = s;
                return true;
        }
        else return false;
}

// calculate remaining sunrise time (if any)
static bool getSunriseSeconds (byte i)
{
        long s = 0;
        s = (currTime - pwmRuntime[i].startTime);
        if (s<0) s+=86400;
        if (s <pwmRuntime[i].sunriseTime)
        {
                pwmRuntime[i].secondsLeft = pwmRuntime[i].sunriseTime - s;
                return true;
        }
        else return false;
}

// main pwm loop
static void pwm ()
{
        if (currentMillis - previousPwmResolution > PWM_RESOLUTION)
        {
                previousPwmResolution = currentMillis;
                getCurrentTime ();
                watts = 0;
                for (byte i = 0; i < PWMS; i++)
                {
                        pwm(i);
                        watts+=pwmRuntime[i].watts;
                        if (watts>MAX_WATTS) max_watts_exceeded=true; else max_watts_exceeded=false;
                }
        }
}
