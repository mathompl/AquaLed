/*
   AQUALED PWM LED support functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#include <pwm.h>

PWM::PWM (_Time *_time)
{
        __time = _time;
}

void PWM::begin ()
{
    #ifndef USE_ADAFRUIT_LIBRARY
            //pwmController.init(B000000); // Address pins A5-A0 set to B000000
            pwmController.init(); // Address pins A5-A0 set to B000000
            pwmController.setPWMFrequency(PWM_I2C_FREQ); // Default is 200Hz, supports 24Hz to 1526Hz
    #else
            pwm_i2c.begin();
            pwm_i2c.setPWMFreq(PWM_I2C_FREQ);
    #endif
            // setup pins
            for (byte i = 0; i < PWMS; i++)
            {
                    initPWM ( i );
                    memset (&__pwmRuntime[i], 0, sizeof __pwmRuntime[i]);
                    updateChannelTimes (i);
                    recoverSunsetAndSunrise (i);
            }
            Wire.setWireTimeout(3000, true);
}

void PWM::initPWM (byte i)
{
        if (pwmSettings[i].isI2C != 0) return;
        pinMode(pwmSettings[i].pin, OUTPUT);
        digitalWrite(pwmSettings[i].pin, OFF);
}

void PWM::updateChannelTimes (byte i)
{
        __pwmRuntime[i].startTime = (long)(pwmSettings[i].onHour) * 60 * 60 + (long)(pwmSettings[i].onMinute)* 60;
        __pwmRuntime[i].stopTime = (long)(pwmSettings[i].offHour) * 60 * 60 + (long)(pwmSettings[i].offMinute)* 60;
        __pwmRuntime[i].sunsetTime =  (long) pwmSettings[i].sunsetLenght * 60;
        __pwmRuntime[i].sunriseTime =  (long)pwmSettings[i].sunriseLenght * 60;
}

void PWM::recoverSunsetAndSunrise (byte i)
{
        if ( getSunsetSeconds (i)  == true)
        {
                __pwmRuntime[i].recoverLastState = true;
                __pwmRuntime[i].valueGoal = (
                        ((double)__pwmRuntime[i].secondsLeft/(double)__pwmRuntime[i].sunsetTime)*
                        (pwmSettings[i].valueDay - getNightValue(i)) +getNightValue(i));
                initDimming (i,abs(__pwmRuntime[i].valueCurrent-__pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
        }
        else
        if ( getSunriseSeconds (i)  == true)
        {
                __pwmRuntime[i].recoverLastState = true;
                __pwmRuntime[i].valueGoal = ((1-(double)__pwmRuntime[i].secondsLeft/(double)__pwmRuntime[i].sunriseTime)*pwmSettings[i].valueDay);
                initDimming (i,abs(__pwmRuntime[i].valueCurrent-__pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
        }
        else __pwmRuntime[i].recoverLastState = false;
}

// sciemnianie/rozjasnianie
void PWM::pwmStep (byte i)
{
        // dimming complete, do nothing
        if (__pwmRuntime[i].valueCurrent == __pwmRuntime[i].valueGoal)
        {
                goalReached (i);
                return;
        }

        if (__pwmRuntime[i].valueGoal > __pwmRuntime[i].valueCurrent)
        {
                __pwmRuntime[i].valueCurrent += __pwmRuntime[i].step;
                if (__pwmRuntime[i].valueCurrent > __pwmRuntime[i].valueGoal) __pwmRuntime[i].valueCurrent=__pwmRuntime[i].valueGoal;
        }
        else
        {
                __pwmRuntime[i].valueCurrent -= __pwmRuntime[i].step;
                if (__pwmRuntime[i].valueCurrent < __pwmRuntime[i].valueGoal) __pwmRuntime[i].valueCurrent=__pwmRuntime[i].valueGoal;
        }
}

void PWM::initDimming (byte i, double dimmingScale, long dimmingTime)
{
        if (__pwmRuntime[i].valueCurrent!=__pwmRuntime[i].valueGoal && __pwmRuntime[i].dimmingStart == false)
        {
                __pwmRuntime[i].dimmingStart = true;
                __pwmRuntime[i].step = (double) ( (double)  dimmingScale  / (double) ((dimmingTime * 1000) * PWM_RESOLUTION_R));
                if ( __pwmRuntime[i].step < PWM_MIN_STEP) __pwmRuntime[i].step = PWM_MIN_STEP;
        }
}

void PWM::goalReached (byte i)
{
        __pwmRuntime[i].dimmingStart = false;
        __pwmRuntime[i].recoverLastState = false;
        __pwmRuntime[i].step = 0;
        __pwmRuntime[i].secondsLeft = 0;
        resetFlags (i);
        forceRefresh = true;
}

void PWM::resetFlags (byte i)
{
        __pwmRuntime[i].isSunset = false;
        __pwmRuntime[i].isSunrise = false;
        forceRefresh = true;
}

void PWM::forcePWMRecovery (boolean test)
{
        for (byte i = 0; i < PWMS; i++)
        {
                if (test && __pwmRuntime[i].testMode!=true) continue;
                resetFlags (i);
                __pwmRuntime[i].dimmingStart = false;
                recoverSunsetAndSunrise(i);
        }
}

void PWM::forceDimmingRestart ()
{
        for (byte i = 0; i < PWMS; i++)
        {
                resetFlags (i);
                __pwmRuntime[i].dimmingStart = false;
        }
}

double PWM::getNightValue (byte i)
{
        double result = 0.0;
        if (pwmSettings[i].isNightLight)
        {
                if (pwmSettings[i].useLunarPhase==0) result = (double)pwmSettings[i].valueNight;
                else result = (double)pwmSettings[i].valueNight * (double)moonPhases[moonPhase] * 0.01;
        }
        return result;
}

// calculate and set pwm value and drive led
void PWM::loop_channel ( byte i )
{
        bool state = getState (i);
        __pwmRuntime[i].isNight= false;

        //test overheat / max watts
        if (lampOverheating == true || max_watts_exceeded == true)
        {
                __pwmRuntime[i].valueCurrent = 0;
                __pwmRuntime[i].dimmingStart = false;
        }
        else
        //test mode
        if (__pwmRuntime[i].testMode)
        {
                __pwmRuntime[i].dimmingStart = false;
        }
        else
        // force off
        if (pwmSettings[i].enabled == 0 || settings.forceOFF == 1)
        {
                __pwmRuntime[i].valueGoal = 0;
                initDimming (i, __pwmRuntime[i].valueCurrent,settings.pwmDimmingTime);
        }
        else
        // force night
        if (settings.forceNight == 1)
        {
                __pwmRuntime[i].valueGoal = 0;
                if (pwmSettings[i].isNightLight)
                        __pwmRuntime[i].valueGoal = getNightValue(i);

                initDimming (i, abs(__pwmRuntime[i].valueCurrent-pwmSettings[i].valueNight), settings.pwmDimmingTime);
                __pwmRuntime[i].isNight= true;
        }
        else
        // ambient/user program
        if (settings.forceAmbient == 1 && pwmSettings[i].isProg &&  pwmSettings[i].isProg == 1)
        {
                __pwmRuntime[i].valueGoal = pwmSettings[i].valueProg;
                initDimming (i, abs(__pwmRuntime[i].valueCurrent-pwmSettings[i].valueProg), settings.pwmDimmingTime);
        }
        else
        if (__pwmRuntime[i].recoverLastState)
        {
                // just dim
        }
        // night light
        else if (!state && pwmSettings[i].isNightLight == 1)
        {
                __pwmRuntime[i].isNight= true;
                __pwmRuntime[i].valueGoal = getNightValue(i);
                initDimming (i, abs(__pwmRuntime[i].valueCurrent-  __pwmRuntime[i].valueGoal),settings.pwmDimmingTime);
        }
        else
        //sunset
        if ( getSunsetSeconds (i) == true)
        {
                __pwmRuntime[i].isSunset = true;
                __pwmRuntime[i].valueGoal = getNightValue(i);
                __pwmRuntime[i].step =  abs(__pwmRuntime[i].valueCurrent-__pwmRuntime[i].valueGoal)  / (double) ((__pwmRuntime[i].secondsLeft * 1000) * PWM_RESOLUTION_R);
                //  if ( __pwmRuntime[i].step < PWM_MIN_STEP) __pwmRuntime[i].step = PWM_MIN_STEP;
        }
        else
        //sunrise
        if ( getSunriseSeconds (i)  == true)
        {
                __pwmRuntime[i].isSunrise = true;
                __pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                __pwmRuntime[i].step =  abs(__pwmRuntime[i].valueCurrent-__pwmRuntime[i].valueGoal)  / (double) ((__pwmRuntime[i].secondsLeft * 1000) * PWM_RESOLUTION_R);
                //if ( pwmRuntime[i].step < PWM_MIN_STEP) pwmRuntime[i].step = PWM_MIN_STEP;
        }
        // day
        else if (state)
        {
                __pwmRuntime[i].valueGoal = pwmSettings[i].valueDay;
                initDimming (i,abs(__pwmRuntime[i].valueCurrent-pwmSettings[i].valueDay),settings.pwmDimmingTime);
        }
        // scheduled off
        else if (!state && pwmSettings[i].isNightLight == 0)
        {
                __pwmRuntime[i].valueGoal = 0;
                initDimming (i, __pwmRuntime[i].valueCurrent,settings.pwmDimmingTime);
        }

        pwmStep (i);


        // no change
        if (__pwmRuntime[i].pwmLast != __pwmRuntime[i].valueCurrent)
        {
                if (pwmSettings[i].enabled && pwmSettings[i].watts && __pwmRuntime[i].valueCurrent)
                {
                        __pwmRuntime[i].watts=(double)(((double)__pwmRuntime[i].valueCurrent / (double)PWM_I2C_MAX) * (double)pwmSettings[i].watts);
                }
                else __pwmRuntime[i].watts=0;
        }

        int val = (int) __pwmRuntime[i].valueCurrent;

        if (pwmSettings[i].isI2C == 0)
        {
                if (pwmSettings[i].invertPwm == 1)
                        val = mapRound(val,  PWM_I2C_MAX, PWM_I2C_MIN,  0, 255);
                else
                        val = mapRound(val, PWM_I2C_MIN, PWM_I2C_MAX,  0, 255);

                // logarithmic dimming table (gamma correction), experimental, works best if max 100%
                      #ifndef NO_DIMMING_TABLE
                if (dimming && settings.softDimming == 1 && (int) val != (int) __pwmRuntime[i].valueGoal)
                        val = dimmingTable[val];
                      #endif
                analogWrite( pwmSettings[i].pin, val);
        }
        else
        {
                // pwm module doesn't need constant updates
                if (__pwmRuntime[i].pwmLast != __pwmRuntime[i].valueCurrent)
                {
                        if (pwmSettings[i].invertPwm == 1)
                                val = mapRound(val, PWM_I2C_MAX, 0.0, PWM_I2C_MIN, PWM_I2C_MAX);

                        PWM_I2C_SETVALUE (pwmSettings[i].pin, val);
                }
        }
        __pwmRuntime[i].pwmLast = __pwmRuntime[i].valueCurrent;
}

// check if scheduled on or off
boolean PWM::getState (byte i)
{
        if (__pwmRuntime[i].stopTime < __pwmRuntime[i].startTime)
        {
                if (__time->getCurrentTime()>=__pwmRuntime[i].startTime && __time->getCurrentTime() < 86400) return true;
                else if (__time->getCurrentTime()>=0 && __time->getCurrentTime() <__pwmRuntime[i].stopTime) return true;
        }
        else
        {
                if (__time->getCurrentTime()>=__pwmRuntime[i].startTime && __time->getCurrentTime() < __pwmRuntime[i].stopTime) return true;
        }
        return false;
}

// calculate remaining sunset time (if any)
long PWM::getSunsetSeconds (byte i)
{
        long s = 0;
        s = __pwmRuntime[i].stopTime - __time->getCurrentTime();
        if (s < 0) s+=86400;
        if (s < __pwmRuntime[i].sunsetTime)
        {
                __pwmRuntime[i].secondsLeft  = s;
                return true;
        }
        else return false;
}

// calculate remaining sunrise time (if any)
long PWM::getSunriseSeconds (byte i)
{
        long s = 0;
        s = (__time->getCurrentTime() - __pwmRuntime[i].startTime);
        if (s<0) s+=86400;
        if (s <__pwmRuntime[i].sunriseTime)
        {
                __pwmRuntime[i].secondsLeft = __pwmRuntime[i].sunriseTime - s;
                return true;
        }
        else return false;
}

PWM_RUNTIME PWM::getRuntime (int pwm)
{
        return __pwmRuntime[pwm];
}

void PWM::setTestMode (byte pwm, bool value)
{
        __pwmRuntime [pwm].testMode = value;

}
void PWM::setnxPwmLast (byte pwm, double value)
{
        __pwmRuntime [pwm].nxPwmLast = value;
}

void PWM::setCurrentValue (byte pwm, double value)
{
        __pwmRuntime [pwm].valueCurrent = value;
}

// main pwm loop
void PWM::loop ()
{
        if (currentMillis - previousPwmResolution > PWM_RESOLUTION)
        {
                previousPwmResolution = currentMillis;
                watts = 0;
                for (byte i = 0; i < PWMS; i++)
                {
                        loop_channel (i);
                        watts+=__pwmRuntime[i].watts;
                        if (watts>MAX_WATTS) max_watts_exceeded=true; else max_watts_exceeded=false;
                }
                Wire.clearWireTimeoutFlag();
        }
}

long PWM::mapRound(long x, long in_min,long in_max, long out_min, long out_max)
{
        return ((x - in_min) * (out_max - out_min) + (in_max - in_min) / 2) / (in_max - in_min) + out_min;
}

double PWM::mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
        return (double)((x - in_min) * ((double)out_max - (double)out_min)  / (in_max - in_min) + (double)out_min);
}
