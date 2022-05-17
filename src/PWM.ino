/**************************************************************
   AQUALED PWM library for controlling LED PWM drivers (c) T. Formanowski 2016-2022
   https://github.com/mathompl/AquaLed
   GNU GENERAL PUBLIC LICENSE
**************************************************************/

#include <pwm.h>

PWM::PWM (_Time *_time)
{
        __time = _time;
}

void PWM::begin ()
{
    #ifndef USE_ADAFRUIT_LIBRARY
        //pwmController.init(B000000); // Address pins A5-A0 set to B000000
        pwmController.init();     // Address pins A5-A0 set to B000000
        pwmController.setPWMFrequency(PWM_I2C_FREQ);     // Default is 200Hz, supports 24Hz to 1526Hz
    #else
        pwm_i2c.begin();
        pwm_i2c.setPWMFreq(PWM_I2C_FREQ);
    #endif
        // setup pins
        for (byte pwmId = 0; pwmId < PWMS; pwmId++)
        {
                initPWM ( pwmId );
                memset (&__pwmRuntime[pwmId], 0, sizeof __pwmRuntime[pwmId]);
                updateChannelTimes (pwmId);
                recoverSunsetAndSunrise (pwmId);
        }
        Wire.setWireTimeout(3000, true);
}

void PWM::initPWM (byte pwmId)
{
        if (pwmSettings[pwmId].isI2C != 0) return;
        pinMode(pwmSettings[pwmId].pin, OUTPUT);
        digitalWrite(pwmSettings[pwmId].pin, OFF);
}

void PWM::updateChannelTimes (byte pwmId)
{
        __pwmRuntime[pwmId].startTime = (long)(pwmSettings[pwmId].onHour) * 60 * 60 + (long)(pwmSettings[pwmId].onMinute)* 60;
        __pwmRuntime[pwmId].stopTime = (long)(pwmSettings[pwmId].offHour) * 60 * 60 + (long)(pwmSettings[pwmId].offMinute)* 60;
        __pwmRuntime[pwmId].sunsetTime =  (long) pwmSettings[pwmId].sunsetLenght * 60;
        __pwmRuntime[pwmId].sunriseTime =  (long)pwmSettings[pwmId].sunriseLenght * 60;
}

void PWM::recoverSunsetAndSunrise (byte pwmId)
{
        if ( getSunsetSeconds (pwmId)  == true)
        {
                __pwmRuntime[pwmId].recoverLastState = true;
                __pwmRuntime[pwmId].valueGoal = (
                        ((double)__pwmRuntime[pwmId].secondsLeft/(double)__pwmRuntime[pwmId].sunsetTime)*
                        (pwmSettings[pwmId].valueDay - getNightValue(pwmId)) +getNightValue(pwmId));
                initDimming (pwmId,abs(__pwmRuntime[pwmId].valueCurrent-__pwmRuntime[pwmId].valueGoal),settings.pwmDimmingTime);
        }
        else
        if ( getSunriseSeconds (pwmId)  == true)
        {
                __pwmRuntime[pwmId].recoverLastState = true;
                __pwmRuntime[pwmId].valueGoal = ((1-(double)__pwmRuntime[pwmId].secondsLeft/(double)__pwmRuntime[pwmId].sunriseTime)*pwmSettings[pwmId].valueDay);
                initDimming (pwmId,abs(__pwmRuntime[pwmId].valueCurrent-__pwmRuntime[pwmId].valueGoal),settings.pwmDimmingTime);
        }
        else __pwmRuntime[pwmId].recoverLastState = false;
}

// sciemnianie/rozjasnianie
void PWM::pwmStep (byte pwmId)
{
        // dimming complete, do nothing
        if (__pwmRuntime[pwmId].valueCurrent == __pwmRuntime[pwmId].valueGoal)
        {
                goalReached (pwmId);
                return;
        }

        if (__pwmRuntime[pwmId].valueGoal > __pwmRuntime[pwmId].valueCurrent)
        {
                __pwmRuntime[pwmId].valueCurrent += __pwmRuntime[pwmId].step;
                if (__pwmRuntime[pwmId].valueCurrent > __pwmRuntime[pwmId].valueGoal) __pwmRuntime[pwmId].valueCurrent=__pwmRuntime[pwmId].valueGoal;
        }
        else
        {
                __pwmRuntime[pwmId].valueCurrent -= __pwmRuntime[pwmId].step;
                if (__pwmRuntime[pwmId].valueCurrent < __pwmRuntime[pwmId].valueGoal) __pwmRuntime[pwmId].valueCurrent=__pwmRuntime[pwmId].valueGoal;
        }
}

void PWM::initDimming (byte pwmId, double dimmingScale, long dimmingTime)
{
        if (__pwmRuntime[pwmId].valueCurrent!=__pwmRuntime[pwmId].valueGoal && __pwmRuntime[pwmId].dimmingStarted== false)
        {
                __pwmRuntime[pwmId].dimmingStarted= true;
                __pwmRuntime[pwmId].step = (double) ( (double)  dimmingScale  / (double) ((dimmingTime * 1000) * PWM_RESOLUTION_R));
                if ( __pwmRuntime[pwmId].step < PWM_MIN_STEP) __pwmRuntime[pwmId].step = PWM_MIN_STEP;
        }
}

void PWM::goalReached (byte pwmId)
{
        __pwmRuntime[pwmId].dimmingStarted= false;
        __pwmRuntime[pwmId].recoverLastState = false;
        __pwmRuntime[pwmId].step = 0;
        __pwmRuntime[pwmId].secondsLeft = 0;
        resetFlags (pwmId);
        forceRefresh = true;
}

void PWM::resetFlags (byte pwmId)
{
        __pwmRuntime[pwmId].isSunset = false;
        __pwmRuntime[pwmId].isSunrise = false;
        forceRefresh = true;
}

void PWM::forcePWMRecovery (boolean test)
{
        for (byte pwmId = 0; pwmId < PWMS; pwmId++)
        {
                if (test && __pwmRuntime[pwmId].testMode!=true) continue;
                resetFlags (pwmId);
                __pwmRuntime[pwmId].dimmingStarted= false;
                recoverSunsetAndSunrise(pwmId);
        }
}

void PWM::forceDimmingRestart ()
{
        for (byte pwmId = 0; pwmId < PWMS; pwmId++)
        {
                resetFlags (pwmId);
                __pwmRuntime[pwmId].dimmingStarted= false;
        }
}

double PWM::getNightValue (byte pwmId)
{
        double result = 0.0;
        if (pwmSettings[pwmId].isNightLight)
        {
                if (pwmSettings[pwmId].useLunarPhase==0) result = (double)pwmSettings[pwmId].valueNight;
                else result = (double)pwmSettings[pwmId].valueNight * (double)__time->getMoonPhaseValue () * 0.01;
        }
        return result;
}

// calculate and set pwm value and drive led
void PWM::loop_channel ( byte pwmId )
{
        bool state = getState (pwmId);
        __pwmRuntime[pwmId].isNight= false;

        //test overheat / max watts
        if (lampOverheating == true || max_watts_exceeded == true)
        {
                __pwmRuntime[pwmId].valueCurrent = 0;
                __pwmRuntime[pwmId].dimmingStarted= false;
        }
        else
        //test mode
        if (__pwmRuntime[pwmId].testMode)
        {
                __pwmRuntime[pwmId].dimmingStarted= false;
        }
        else
        // force off
        if (pwmSettings[pwmId].enabled == 0 || settings.forceOFF == 1)
        {
                __pwmRuntime[pwmId].valueGoal = 0;
                initDimming (pwmId, __pwmRuntime[pwmId].valueCurrent,settings.pwmDimmingTime);
        }
        else
        // force night
        if (settings.forceNight == 1)
        {
                __pwmRuntime[pwmId].valueGoal = 0;
                if (pwmSettings[pwmId].isNightLight)
                        __pwmRuntime[pwmId].valueGoal = getNightValue(pwmId);

                initDimming (pwmId, abs(__pwmRuntime[pwmId].valueCurrent-pwmSettings[pwmId].valueNight), settings.pwmDimmingTime);
                __pwmRuntime[pwmId].isNight= true;
        }
        else
        // ambient/user program
        if (settings.forceAmbient == 1 && pwmSettings[pwmId].isProg &&  pwmSettings[pwmId].isProg == 1)
        {
                __pwmRuntime[pwmId].valueGoal = pwmSettings[pwmId].valueProg;
                initDimming (pwmId, abs(__pwmRuntime[pwmId].valueCurrent-pwmSettings[pwmId].valueProg), settings.pwmDimmingTime);
        }
        else
        if (__pwmRuntime[pwmId].recoverLastState)
        {
                // just dim
        }
        // night light
        else if (!state && pwmSettings[pwmId].isNightLight == 1)
        {
                __pwmRuntime[pwmId].isNight= true;
                __pwmRuntime[pwmId].valueGoal = getNightValue(pwmId);
                initDimming (pwmId, abs(__pwmRuntime[pwmId].valueCurrent-  __pwmRuntime[pwmId].valueGoal),settings.pwmDimmingTime);
        }
        else
        //sunset
        if ( getSunsetSeconds (pwmId) == true)
        {
                __pwmRuntime[pwmId].isSunset = true;
                __pwmRuntime[pwmId].valueGoal = getNightValue(pwmId);
                __pwmRuntime[pwmId].step =  abs(__pwmRuntime[pwmId].valueCurrent-__pwmRuntime[pwmId].valueGoal)  / (double) ((__pwmRuntime[pwmId].secondsLeft * 1000) * PWM_RESOLUTION_R);
                //  if ( __pwmRuntime[pwmId].step < PWM_MIN_STEP) __pwmRuntime[pwmId].step = PWM_MIN_STEP;
        }
        else
        //sunrise
        if ( getSunriseSeconds (pwmId)  == true)
        {
                __pwmRuntime[pwmId].isSunrise = true;
                __pwmRuntime[pwmId].valueGoal = pwmSettings[pwmId].valueDay;
                __pwmRuntime[pwmId].step =  abs(__pwmRuntime[pwmId].valueCurrent-__pwmRuntime[pwmId].valueGoal)  / (double) ((__pwmRuntime[pwmId].secondsLeft * 1000) * PWM_RESOLUTION_R);
                //if ( pwmRuntime[pwmId].step < PWM_MIN_STEP) pwmRuntime[pwmId].step = PWM_MIN_STEP;
        }
        // day
        else if (state)
        {
                __pwmRuntime[pwmId].valueGoal = pwmSettings[pwmId].valueDay;
                initDimming (pwmId,abs(__pwmRuntime[pwmId].valueCurrent-pwmSettings[pwmId].valueDay),settings.pwmDimmingTime);
        }
        // scheduled off
        else if (!state && pwmSettings[pwmId].isNightLight == 0)
        {
                __pwmRuntime[pwmId].valueGoal = 0;
                initDimming (pwmId, __pwmRuntime[pwmId].valueCurrent,settings.pwmDimmingTime);
        }

        pwmStep (pwmId);


        // no change
        if (__pwmRuntime[pwmId].valueLast != __pwmRuntime[pwmId].valueCurrent)
        {
                if (pwmSettings[pwmId].enabled && pwmSettings[pwmId].watts && __pwmRuntime[pwmId].valueCurrent)
                {
                        __pwmRuntime[pwmId].watts=(double)(((double)__pwmRuntime[pwmId].valueCurrent / (double)PWM_I2C_MAX) * (double)pwmSettings[pwmId].watts);
                }
                else __pwmRuntime[pwmId].watts=0;
        }

        int val = (int) __pwmRuntime[pwmId].valueCurrent;

        if (pwmSettings[pwmId].isI2C == 0)
        {
                if (pwmSettings[pwmId].invertPwm == 1)
                        val = mapRound(val,  PWM_I2C_MAX, PWM_I2C_MIN,  0, 255);
                else
                        val = mapRound(val, PWM_I2C_MIN, PWM_I2C_MAX,  0, 255);

                // logarithmic dimming table (gamma correction), experimental, works best if max 100%
                      #ifndef NO_DIMMING_TABLE
                if (dimming && settings.softDimming == 1 && (int) val != (int) __pwmRuntime[pwmId].valueGoal)
                        val = dimmingTable[val];
                      #endif
                analogWrite( pwmSettings[pwmId].pin, val);
        }
        else
        {
                // pwm module doesn't need constant updates
                if (__pwmRuntime[pwmId].valueLast != __pwmRuntime[pwmId].valueCurrent)
                {
                        if (pwmSettings[pwmId].invertPwm == 1)
                                val = mapRound(val, PWM_I2C_MAX, 0.0, PWM_I2C_MIN, PWM_I2C_MAX);

                        PWM_I2C_SETVALUE (pwmSettings[pwmId].pin, val);
                }
        }
        __pwmRuntime[pwmId].valueLast = __pwmRuntime[pwmId].valueCurrent;
}

// check if scheduled on or off
boolean PWM::getState (byte pwmId)
{
        if (__pwmRuntime[pwmId].stopTime < __pwmRuntime[pwmId].startTime)
        {
                if (__time->getCurrentTime()>=__pwmRuntime[pwmId].startTime && __time->getCurrentTime() < 86400) return true;
                else if (__time->getCurrentTime()>=0 && __time->getCurrentTime() <__pwmRuntime[pwmId].stopTime) return true;
        }
        else
        {
                if (__time->getCurrentTime()>=__pwmRuntime[pwmId].startTime && __time->getCurrentTime() < __pwmRuntime[pwmId].stopTime) return true;
        }
        return false;
}

// calculate remaining sunset time (if any)
long PWM::getSunsetSeconds (byte pwmId)
{
        long s = 0;
        s = __pwmRuntime[pwmId].stopTime - __time->getCurrentTime();
        if (s < 0) s+=86400;
        if (s < __pwmRuntime[pwmId].sunsetTime)
        {
                __pwmRuntime[pwmId].secondsLeft  = s;
                return true;
        }
        else return false;
}

// calculate remaining sunrise time (if any)
long PWM::getSunriseSeconds (byte pwmId)
{
        long s = 0;
        s = (__time->getCurrentTime() - __pwmRuntime[pwmId].startTime);
        if (s<0) s+=86400;
        if (s <__pwmRuntime[pwmId].sunriseTime)
        {
                __pwmRuntime[pwmId].secondsLeft = __pwmRuntime[pwmId].sunriseTime - s;
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
void PWM::setValueLastNextion (byte pwm, double value)
{
        __pwmRuntime [pwm].valueLastNextion = value;
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
                for (byte pwmId = 0; pwmId < PWMS; pwmId++)
                {
                        loop_channel (pwmId);
                        watts+=__pwmRuntime[pwmId].watts;
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
