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
                pwmRuntime[i].valueCurrent = 0.0;
                pwmRuntime[i].valueTest = 0;
                pwmRuntime[i].dimmingStart = false;
                pwmRuntime[i].isSunset = false;
                pwmRuntime[i].isSunrise = false;
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
                pwmRuntime[i].valueGoal = (pwmRuntime[i].sunsetValue*pwmSettings[i].valueDay);
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal),settings.pwmDimmingTime);

        }
        else
        if ( getSunriseSeconds (i)  == true)
        {
                pwmRuntime[i].recoverLastState = true;
                pwmRuntime[i].valueGoal = (pwmRuntime[i].sunriseValue*pwmSettings[i].valueDay);
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
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
static boolean pwmStep (byte i)
{
        // dimming complete, do nothing
        if (pwmRuntime[i].valueCurrent == pwmRuntime[i].valueGoal)
        {
                goalReached (i);
                return false;
        }

        if (pwmRuntime[i].valueGoal > pwmRuntime[i].valueCurrent)
        {
                pwmRuntime[i].valueCurrent = pwmRuntime[i].valueCurrent +  pwmRuntime[i].step;
                if (pwmRuntime[i].valueCurrent >= pwmRuntime[i].valueGoal
                    || pwmRuntime[i].valueCurrent+0.1 >=pwmRuntime[i].valueGoal ) pwmRuntime[i].valueCurrent = pwmRuntime[i].valueGoal;
        }
        else
        {
                pwmRuntime[i].valueCurrent = pwmRuntime[i].valueCurrent -  pwmRuntime[i].step;
                if (pwmRuntime[i].valueCurrent <= 0 || pwmRuntime[i].valueCurrent < 0.1) pwmRuntime[i].valueCurrent = 0;
        }
        return true;
}

static void goalReached (byte i)
{
        pwmRuntime[i].valueCurrent = pwmRuntime[i].valueGoal;
        pwmRuntime[i].dimmingStart = false;
        pwmRuntime[i].recoverLastState = false;
        pwmRuntime[i].isSunset= false;
        pwmRuntime[i].isSunrise= false;
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
        if (pwmRuntime[i].valueCurrent!=pwmRuntime[i].valueGoal && pwmRuntime[i].dimmingStart == false)
        {
                pwmRuntime[i].dimmingStart = true;
                pwmRuntime[i].step = (double) ( (double)  dimmingScale  / (double) ((dimmingTime * 1000) / PWM_RESOLUTION));
                if ( pwmRuntime[i].step < PWM_MIN_STEP) pwmRuntime[i].step = PWM_MIN_STEP;
        }
}

// calculate and set pwm value and drive led
static void pwm( byte i )
{
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
                pwmRuntime[i].isSunrise= false;
                pwmRuntime[i].isSunset= false;
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
                pwmRuntime[i].isSunrise= false;
                pwmRuntime[i].isSunset= false;
        }
        else
        // ambient/user program
        if (settings.forceAmbient == 1 && pwmSettings[i].isProg &&  pwmSettings[i].isProg == 1)
        {
                pwmRuntime[i].isSunrise= false;
                pwmRuntime[i].isSunset= false;
                pwmRuntime[i].valueGoal = pwmSettings[i].valueProg;
                initDimming (i, abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueProg), settings.pwmDimmingTime);
        }
        else
        if (pwmRuntime[i].recoverLastState)
        {
                pwmRuntime[i].isSunrise= false;
                pwmRuntime[i].isSunset= false;
        }
        // night light
        else if (!state && pwmSettings[i].isNightLight == 1)
        {
                pwmRuntime[i].isNight= true;
                pwmRuntime[i].isSunrise= false;
                pwmRuntime[i].isSunset= false;
                pwmRuntime[i].valueGoal = getNightValue(i);
                initDimming (i, abs(pwmRuntime[i].valueCurrent-  pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
        }
        else
        //sunset
        if ( pwmRuntime[i].isSunset || getSunsetSeconds (i) == true)
        {
                pwmRuntime[i].isSunset = true;
                pwmRuntime[i].valueGoal = getNightValue(i);
                initDimming (i,pwmRuntime[i].valueCurrent-pwmRuntime[i].valueGoal,pwmRuntime[i].sunsetSecondsLeft);
        }
        else
        //sunrise
        if ( pwmRuntime[i].isSunrise || getSunriseSeconds (i)  == true)
        {
                pwmRuntime[i].isSunrise = true;
                pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                initDimming (i,abs(pwmRuntime[i].valueCurrent-(double)pwmSettings[i].valueDay),pwmRuntime[i].sunriseSecondsLeft);
        }
        // day
        else if (state)
        {
                pwmRuntime[i].isSunrise= false;
                pwmRuntime[i].isSunset= false;
                pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                initDimming (i,abs(pwmRuntime[i].valueCurrent-pwmSettings[i].valueDay),settings.pwmDimmingTime);
        }
        // scheduled off
        else if (!state && pwmSettings[i].isNightLight == 0)
        {
                pwmRuntime[i].isSunrise= false;
                pwmRuntime[i].isSunset= false;
                pwmRuntime[i].valueGoal = 0;
                initDimming (i, pwmRuntime[i].valueCurrent,settings.pwmDimmingTime);
        }

        pwmStep (i);

        // no change
        if (pwmRuntime[i].pwmLast == pwmRuntime[i].valueCurrent)
        {
                pwmRuntime[i].hasChanged = false;
        }
        else
        {
                pwmRuntime[i].hasChanged = true;
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
