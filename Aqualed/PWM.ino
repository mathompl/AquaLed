/*
   AQUALED PWM LED support functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#include <Arduino.h>

static void setupPWMPins ()
{
        currTime = (long)hour ()* 60 * 60 + (long)minute () * 60 + (long)second ();
        // setup pins
        for (byte i = 0; i < PWMS; i++)
        {
                initPWM ( i );
                pwmRuntime[i].valueCurrent = 0;
                pwmRuntime[i].valueTest = 0;
                pwmRuntime[i].dimmingStart = false;
                updateChannelTimes (i);
                recoverSunsetAndSunrise (i);
                pwmRuntime[i].testMode = false;
        }
        pwm_i2c.begin();
        pwm_i2c.setPWMFreq(PWM_I2C_FREQ);
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
                pwmRuntime[i].valueGoal = (int) (pwmRuntime[i].sunsetValue*pwmSettings[i].valueDay);
                pwmRuntime[i].dimmingStart = true;
                pwmRuntime[i].dimmingScale =  abs(pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal);

        }
        else
        if ( getSunriseSeconds (i)  == true)
        {
                pwmRuntime[i].recoverLastState = true;
                pwmRuntime[i].valueGoal = (int) (pwmRuntime[i].sunriseValue*pwmSettings[i].valueDay);
                pwmRuntime[i].dimmingStart = true;
                pwmRuntime[i].dimmingScale = abs(pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal);
        }
        else pwmRuntime[i].recoverLastState = false;
}

static void initPWM (byte i)
{
        if (pwmSettings[i].isI2C != 0) return;
        pinMode(pwmSettings[i].pin, OUTPUT);
        digitalWrite(pwmSettings[i].pin, OFF);
}

// sciemnianie/rozjasnianie
static boolean pwmStep (byte i, long dimmingTime)
{

        // dimming complete, do nothing
        if (pwmRuntime[i].valueCurrent == pwmRuntime[i].valueGoal)
        {
                goalReached (i);
                return false;
        }
        double step;

        double valueCurrent = pwmRuntime[i].valueCurrent;

// full pwm scale dimming when no other data
// scale dimming
        if (pwmRuntime[i].dimmingStart == true || abs (pwmRuntime[i].valueCurrent - pwmRuntime[i].valueGoal) < 0.01)
        {
                step = (double) ( (double)  pwmRuntime[i].dimmingScale  / (double) ((dimmingTime * 1000) / PWM_RESOLUTION));
        }
        if (step < PWM_MIN_STEP) step = PWM_MIN_STEP;

        if (pwmRuntime[i].valueGoal > valueCurrent)
        {
                valueCurrent = valueCurrent + step;
                if (valueCurrent >= pwmRuntime[i].valueGoal || valueCurrent+0.1 >=pwmRuntime[i].valueGoal ) valueCurrent = pwmRuntime[i].valueGoal;
        }
        else
        {
                valueCurrent = valueCurrent - step;
                if (valueCurrent <= 0 || valueCurrent < 0.1) valueCurrent = 0;
        }
        pwmRuntime[i].valueCurrent = valueCurrent;
        return true;
}

static void goalReached (byte i)
{
        pwmRuntime[i].valueCurrent = pwmRuntime[i].valueGoal;
        pwmRuntime[i].dimmingStart = false;
        pwmRuntime[i].recoverLastState = false;
}

// calculate when dimming actualy starts to scale dimming values
static bool isDimmingStart (byte i)
{
        if (pwmRuntime[i].valueGoal != pwmRuntime[i].valueCurrent  && !pwmRuntime[i].dimmingStart) return true;
        else return false;
}

static void forcePWMRecovery ()
{
        for (byte i = 0; i < PWMS; i++)
        {
                recoverSunsetAndSunrise(i);
        }
}

static void forceDimmingRestart ()
{
        for (byte i = 0; i < PWMS; i++)
        {
                pwmRuntime[i].dimmingStart = false;
        }
}

static double getNightValue (byte i)
{
        double result = 0.0;
        if (pwmSettings[i].isNightLight)
        {
                if (pwmSettings[i].useLunarPhase==0) result = (double)pwmSettings[i].valueNight;
                else result = (double)pwmSettings[i].valueNight * (double)moonPhases[moonPhase]/(double)100;
        }
        return result;
}

static void initDimming (byte i, double dimmingScale, long dimmingTime)
{
        if (isDimmingStart(i))
        {
                pwmRuntime[i].dimmingStart = true;
                pwmRuntime[i].dimmingScale = dimmingScale;
                if (dimmingTime>0) pwmRuntime[i].dimmingTime = dimmingTime;
        }
}

// calculate and set pwm value and drive led
static void pwm( byte i )
{
        pwmRuntime[i].isSunset= false;
        pwmRuntime[i].isSunrise= false;
        pwmRuntime[i].isNight= false;
        bool state = getState (i);

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
                pwmRuntime[i].valueCurrent = pwmRuntime[i].valueTest;
                pwmRuntime[i].dimmingStart = false;
        }
        else
        // force off
        if (pwmSettings[i].enabled == 0 || settings.forceOFF == 1)
        {
                pwmRuntime[i].valueGoal = 0;
                initDimming (i, pwmRuntime[i].valueCurrent,-1);
                pwmStep (i, settings.pwmDimmingTime);
        }
        else
        // force night
        if (settings.forceNight == 1)
        {
                pwmRuntime[i].valueGoal = 0;
                if (pwmSettings[i].isNightLight)
                        pwmRuntime[i].valueGoal = getNightValue(i);

                initDimming (i, abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueNight),-1);
                pwmRuntime[i].isNight= true;
                pwmStep (i, settings.pwmDimmingTime );
        }
        else
        // ambient/user program
        if (settings.forceAmbient == 1 && pwmSettings[i].isProg &&  pwmSettings[i].isProg == 1)
        {
                pwmRuntime[i].valueGoal = pwmSettings[i].valueProg;
                initDimming (i, abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueProg),-1);
                pwmStep (i, settings.pwmDimmingTime);
        }
        else
        if (pwmRuntime[i].recoverLastState)
        {
                pwmStep (i,settings.pwmDimmingTime);
        }
        // night light
        else if (!state && pwmSettings[i].isNightLight == 1)
        {
                pwmRuntime[i].isNight= true;
                pwmRuntime[i].valueGoal = getNightValue(i);
                initDimming (i, abs(pwmRuntime[i].valueCurrent-  pwmRuntime[i].valueGoal),-1);
                pwmStep (i, settings.pwmDimmingTime);
        }
        else
        //sunset
        if ( getSunsetSeconds (i) == true)
        {
                pwmRuntime[i].isSunset = true;
                pwmRuntime[i].valueGoal = getNightValue(i);
                initDimming (i,pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal,pwmRuntime[i].sunsetSecondsLeft);
                pwmStep (i, pwmRuntime[i].dimmingTime);
        }
        else
        //sunrise
        if ( getSunriseSeconds (i)  == true)
        {
                pwmRuntime[i].isSunrise = true;
                pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                initDimming (i,abs(pwmRuntime[i].valueCurrent-(double)pwmSettings[i].valueDay),pwmRuntime[i].sunriseSecondsLeft);
                pwmStep (i, pwmRuntime[i].dimmingTime);
        }
        // day
        else if (state)
        {
                pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueDay),-1);
                pwmStep (i,settings.pwmDimmingTime);
        }
        // scheduled off
        else if (!state && pwmSettings[i].isNightLight == 0)
        {
                pwmRuntime[i].valueGoal = 0;
                initDimming (i, pwmRuntime[i].valueCurrent,-1);
                pwmStep (i,settings.pwmDimmingTime);
        }

        // no change
        if (pwmRuntime[i].pwmLast == pwmRuntime[i].valueCurrent) return;

        int val = (int) pwmRuntime[i].valueCurrent;

        if (pwmSettings[i].isI2C == 0)
        {
                if (pwmSettings[i].invertPwm == 1)
                        val = mapRound(val,  PWM_I2C_MAX, PWM_I2C_MIN,  0, 255);
                else
                        val = mapRound(val, PWM_I2C_MIN, PWM_I2C_MAX,  0, 255);
                // logarithmic dimming table, experimental, longs best if max 100%
                #ifndef NO_DIMMING_TABLE
                if (dimming && settings.softDimming == 1 && (int) val != (int) pwmRuntime[i].valueGoal)
                        val = dimmingTable[val];
                #endif
                analogWrite( pwmSettings[i].pin, val);
        }
        else
        {
                // pwm module does not need constant updates
                if (pwmRuntime[i].pwmLast != pwmRuntime[i].valueCurrent)
                {
                        if (pwmSettings[i].invertPwm == 1)
                                val = mapRound(val, PWM_I2C_MAX, 0.0, PWM_I2C_MIN, PWM_I2C_MAX);

                        pwm_i2c.setPWM(pwmSettings[i].pin, 0, val );
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
        pwmRuntime[i].sunsetSecondsLeft = pwmRuntime[i].stopTime - currTime;
        if (pwmRuntime[i].sunsetSecondsLeft < 0) pwmRuntime[i].sunsetSecondsLeft+=86400;
        if (pwmRuntime[i].sunsetSecondsLeft < pwmRuntime[i].sunsetTime)
        {
                pwmRuntime[i].sunsetValue  = (double)pwmRuntime[i].sunsetSecondsLeft/(double)pwmRuntime[i].sunsetTime;
                return true;
        }
        else return false;
}

// calculate remaining sunrise time (if any)
static bool getSunriseSeconds (byte i)
{
        pwmRuntime[i].sunriseSecondsLeft = (currTime - pwmRuntime[i].startTime);
        if (pwmRuntime[i].sunriseSecondsLeft<0) return false;
        if (pwmRuntime[i].sunriseSecondsLeft > 86400) pwmRuntime[i].sunriseSecondsLeft-=86400;
        if (pwmRuntime[i].sunriseSecondsLeft <pwmRuntime[i].sunriseTime)
        {
                pwmRuntime[i].sunriseValue  = (double)pwmRuntime[i].sunriseSecondsLeft/(double)pwmRuntime[i].sunriseTime;
                pwmRuntime[i].sunriseSecondsLeft = pwmRuntime[i].sunriseTime - pwmRuntime[i].sunriseSecondsLeft;
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
                currTime = (long)hour ()* 60 * 60 + (long)minute () * 60 + (long)second ();
                for (byte i = 0; i < PWMS; i++)
                        pwm(i);
        }
}

static long mapRound(long x, long in_min,long in_max, long out_min, long out_max)
{
        return ((x - in_min) * (out_max - out_min) + (in_max - in_min) / 2) / (in_max - in_min) + out_min;
}

static double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
        return (double)((x - in_min) * ((double)out_max - (double)out_min)  / (in_max - in_min) + (double)out_min);
}
