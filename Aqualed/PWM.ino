/*
   AQUALED PWM LED support functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#include <Arduino.h>

void setupPWMPins ()
{
        // setup pins
        for (byte i = 0; i < PWMS; i++)
        {
                initPWM ( i );
                pwmChannel[i].valueCurrent = 0;
                pwmChannel[i].valueTest = 0;
                pwmChannel[i].dimmingStart = false;
                pwmChannel[i].recoverLastState = false;
                pwmChannel[i].testMode = false;
                recovery = true;
        }
        pwm_i2c.begin();
        pwm_i2c.setPWMFreq(PWM_I2C_FREQ);
}

void initPWM (byte i)
{
        if (pwmChannel[i].isI2C != 0) return;

        pinMode(pwmChannel[i].pin, OUTPUT);
        digitalWrite(pwmChannel[i].pin, OFF);
}

// sciemnianie/rozjasnianie
boolean pwmStep (byte i, long dimmingTime)
{
        boolean dimming = false;
        double valueCurrent = pwmChannel[i].valueCurrent;
        byte valueGoal = pwmChannel[i].valueGoal;

        // dimming complete, do nothing
        if (valueCurrent == valueGoal)
        {
                pwmChannel[i].dimmingStart = false;
                pwmChannel[i].recoverLastState = false;
                return dimming;
        }



        double step;

// full pwm scale dimming when no other data
        double max = 255;

// scale dimming
        if (pwmChannel[i].dimmingStart == true)
        {
                max = (double) pwmChannel[i].dimmingScale;
                step = (double) ( (double) max / (double) ((dimmingTime * 1000) / PWM_RESOLUTION));
        }
        if (step < PWM_MIN_STEP) step = PWM_MIN_STEP;
        byte stepsLeft = (valueGoal - valueCurrent) / step;
        if (stepsLeft < 0) stepsLeft *= -1;
        if (valueGoal > valueCurrent)
        {
                valueCurrent = valueCurrent + step;
                if (valueCurrent >= valueGoal || valueCurrent+0.1 >=valueGoal ) valueCurrent = valueGoal;
        }
        else
        {
                valueCurrent = valueCurrent - step;
                if (valueCurrent <= 0 || valueCurrent < 0.1) valueCurrent = 0;
        }

        pwmChannel[i].valueCurrent = valueCurrent;

        if (pwmLast[i] > pwmChannel[i].valueCurrent && pwmChannel[i].valueCurrent <= pwmChannel[i].valueGoal)
        {
            pwmChannel[i].valueCurrent = valueGoal;
        }

        if (pwmLast[i] < pwmChannel[i].valueCurrent && pwmChannel[i].valueCurrent >= pwmChannel[i].valueGoal)
        {
            pwmChannel[i].valueCurrent = valueGoal;
        }


        return true;
}

// calculate when dimming actualy starts to scale dimming values
static bool isDimmingStart (byte i)
{
        if (pwmChannel[i].valueGoal != pwmChannel[i].valueCurrent  && !pwmChannel[i].dimmingStart) return true;
        else return false;
}

static void forcePWMRecovery ()
{
        recovery = true;
}

// calculate and set pwm value and drive led
static void pwm( byte i )
{
        pwmChannel[i].isSunset= false;
        pwmChannel[i].isSunrise= false;
        pwmChannel[i].isNight= false;
        boolean dimming = false;
        long ssMillis;
        long srMillis;
        bool state = getState (i);

        //test mode
        if (pwmChannel[i].testMode)
        {
                pwmChannel[i].valueCurrent = pwmChannel[i].valueTest;
                pwmChannel[i].recoverLastState = 0;
                pwmChannel[i].dimmingStart = false;
        }
        else
        // force off
        if (pwmChannel[i].enabled == 0 || SETTINGS.forceOFF == 1)
        {
                pwmChannel[i].valueGoal = 0;
                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = pwmChannel[i].valueCurrent;
                        pwmChannel[i].recoverLastState = 0;
                }

                dimming = pwmStep (i, SETTINGS.pwmDimmingTime);
        }
        else
        // force night
        if (SETTINGS.forceNight == 1)
        {
                if (pwmChannel[i].isNightLight)
                {
                        pwmChannel[i].valueGoal = pwmChannel[i].valueNight;
                }
                else
                        pwmChannel[i].valueGoal = 0;

                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].recoverLastState = 0;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].valueCurrent-pwmChannel[i].valueNight);
                }
                pwmChannel[i].isNight= true;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime );
        }
        else
        // ambient/user program
        if (SETTINGS.forceAmbient == 1 && pwmChannel[i].isProg &&  pwmChannel[i].isProg == 1)
        {
                pwmChannel[i].valueGoal = pwmChannel[i].valueProg;
                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].recoverLastState = 0;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].valueCurrent-pwmChannel[i].valueProg);
                }

                dimming = pwmStep (i, SETTINGS.pwmDimmingTime);
        }
        else
        if (pwmChannel[i].recoverLastState)
        {
                dimming = pwmStep (i,SETTINGS.pwmDimmingTime);
        }
        // night light
        else if (!state && pwmChannel[i].isNightLight == 1)
        {
                pwmChannel[i].valueGoal = pwmChannel[i].valueNight;
                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].valueCurrent-pwmChannel[i].valueNight);
                }
                pwmChannel[i].isNight= true;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime);
        }
        else
        //sunset
        if ( getSunsetMillis (i, ssMillis) > 0)
        {
                if (recovery)
                {
                        pwmChannel[i].valueGoal =  (byte) (( (ssMillis/1000/60) * pwmChannel[i].valueDay) / pwmChannel[i].sunsetLenght);
                        //if (pwmChannel[i].isNightLight && pwmChannel[i].valueGoal <= pwmChannel[i].valueNight) pwmChannel[i].valueGoal = pwmChannel[i].valueNight;
                        if (pwmChannel[i].isNightLight == 1) pwmChannel[i].valueGoal = pwmChannel[i].valueNight;
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].recoverLastState = true;

                        pwmChannel[i].dimmingScale = abs (pwmChannel[i].valueCurrent-pwmChannel[i].valueGoal);
                        return;
                }
                else
                {
                        pwmChannel[i].isSunset = true;
                        if (pwmChannel[i].isNightLight == 1) pwmChannel[i].valueGoal = pwmChannel[i].valueNight; else pwmChannel[i].valueGoal = 0;

                        if (isDimmingStart(i))
                        {
                                pwmChannel[i].dimmingStart = true;
                                pwmChannel[i].dimmingScale = abs(pwmChannel[i].valueCurrent-pwmChannel[i].valueGoal);
                                pwmChannel[i].dimmingTime = ssMillis;
                        }
                }
                dimming = pwmStep (i, pwmChannel[i].dimmingTime / 1000);
        }
        else
        //sunrise
        if ( getSunriseMillis(i, srMillis) > 0)
        {
                // restore state after shutdown or forced mode
                if (recovery)
                {
                        pwmChannel[i].valueGoal =  (byte) ( abs( pwmChannel[i].valueDay - (pwmChannel[i].sunriseLenght - srMillis/1000/60) * pwmChannel[i].valueDay) / pwmChannel[i].sunriseLenght);
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].recoverLastState = true;
                    //    pwmChannel[i].dimmingScale = pwmChannel[i].valueGoal;
                        pwmChannel[i].dimmingScale = abs (pwmChannel[i].valueCurrent-pwmChannel[i].valueGoal);
                        return;
                }
                else
                {
                        pwmChannel[i].isSunrise = true;
                        pwmChannel[i].valueGoal = pwmChannel[i].valueDay;

                        if (isDimmingStart(i))
                        {
                                pwmChannel[i].dimmingStart = true;
                                pwmChannel[i].dimmingScale = abs(pwmChannel[i].valueCurrent-pwmChannel[i].valueDay);
                                pwmChannel[i].dimmingTime = srMillis;
                        }
                }
                dimming = pwmStep (i, pwmChannel[i].dimmingTime  / 1000);
        }
        else if (state)
        {
                pwmChannel[i].valueGoal = pwmChannel[i].valueDay;

                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].valueCurrent-pwmChannel[i].valueDay);
                }

                dimming = pwmStep (i,SETTINGS.pwmDimmingTime);
        }
        // scheduled off
        else if (!state && pwmChannel[i].isNightLight == 0)
        {
                pwmChannel[i].valueGoal = 0;

                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = pwmChannel[i].valueCurrent;
                }
                dimming = pwmStep (i,SETTINGS.pwmDimmingTime);
        }

        // no change
        if (pwmLast[i] == pwmChannel[i].valueCurrent) return;

        if (pwmChannel[i].isI2C == 0)
        {
                byte val = (byte) pwmChannel[i].valueCurrent;
                // logarithmic dimming table, experimental, longs best if max 100%
                if (dimming && SETTINGS.softDimming == 1 && (byte) val != pwmChannel[i].valueGoal)
                {
                        //val = (byte)pgm_read_byte(&dimmingTable[val]);
                        val = dimmingTable[val];
                }
                if (pwmChannel[i].invertPwm == 1)
                        analogWrite( pwmChannel[i].pin,  mapDoubleToInt (val, 255.0,0.0, 0,255));
                else
                        analogWrite( pwmChannel[i].pin,  val);
        }
        else
        {
                // pwm module does not need constant updates
                if (pwmLast[i] != pwmChannel[i].valueCurrent)
                {
                        unsigned int v;
                        if (pwmChannel[i].invertPwm == 1)
                                v = mapDoubleToInt(pwmChannel[i].valueCurrent, 255.0, 0.0, PWM_I2C_MIN, PWM_I2C_MAX);
                        else
                                v = mapDoubleToInt(pwmChannel[i].valueCurrent, 0.0, 255.0, PWM_I2C_MIN, PWM_I2C_MAX);
                        pwm_i2c.setPWM(pwmChannel[i].pin, 0, v );
                }
        }
    

        pwmLast[i] = pwmChannel[i].valueCurrent;
}

// check if scheduled on or off
bool getState (byte i)
{
        unsigned long currTime = 0;
        unsigned long startTime = 0;
        unsigned long stopTime = 0;
        boolean midnight = false;
        startTime = (long)(pwmChannel[i].onHour) * 60 * 60 + (long)(pwmChannel[i].onMinute)* 60 ;
        stopTime = (long)(pwmChannel[i].offHour) * 60 * 60 + (long)(pwmChannel[i].offMinute)* 60 ;
        currTime = (long)hour ()* 60 * 60 + (long)minute () * 60 + (long)second ();

        if (stopTime < startTime) midnight = true;

        if (!midnight)
        {
                if (currTime>=startTime && currTime < stopTime) return true;
        }
        else
        {
                if (currTime>=startTime && currTime < 86400) return true;
                else if (currTime>=0 && currTime < stopTime) return true;
        }

        return false;
}

// calculate remaining sunset time (if any)
long getSunsetMillis (byte i, long &m)
{
        unsigned long currTime = 0;
        unsigned long startTime = 0;
        unsigned long stopTime = 0;
        boolean midnight = false;
        stopTime = (long)pwmChannel[i].offHour * 60 * 60 + (long)pwmChannel[i].offMinute * 60 ;
        currTime = (long)hour () * 60 * 60 + (long)minute () * 60 + (long)second ();
        startTime = stopTime - (long) pwmChannel[i].sunsetLenght * 60 ;

        if (startTime < 0) midnight = true;

        // before midnight
        if (currTime >= startTime && currTime <= stopTime)
        {
                m = (long) (stopTime - currTime) * 1000;
                return abs(m);
        }

        // midnight crossing
        if (midnight)
        {
                stopTime += 86400;
                startTime = stopTime - (long) pwmChannel[i].sunsetLenght * 60;

                if (currTime >= startTime && currTime <= stopTime)
                {
                        m = (long) (stopTime - currTime) * 1000;
                        return abs(m);
                }
        }

        m = 0;
        return abs (m);

}

// calculate remaining sunrise time (if any)
long getSunriseMillis (byte i, long &m)
{
        unsigned long currTime = 0;
        unsigned long startTime = 0;
        unsigned long stopTime = 0;
        // in seconds
        startTime = (long)pwmChannel[i].onHour * 60* 60  + (long)pwmChannel[i].onMinute* 60 ;
        currTime = (long)hour () * 60 * 60 + (long)minute () * 60 + (long) second();
        stopTime = startTime + (long)pwmChannel[i].sunriseLenght * 60;

        boolean midnight = false;

        // check if midnight cross
        if (stopTime > 86400) midnight = true;

        // before midnight
        if (currTime >= startTime && currTime <= stopTime)
        {
                m = (long) (stopTime - currTime) * 1000;
                return abs(m);
        }

        // if crossing midnight
        if (midnight)
        {
                currTime += 86400;
                if (currTime >= startTime && currTime <= stopTime)
                {
                        m = (long) (stopTime - currTime) * 1000;
                        return abs(m);
                }
        }
        m = 0;
        return abs(m);
}

// main pwm loop
void pwm ()
{
        if (currentMillis - previousPwmResolution > PWM_RESOLUTION)
        {
                previousPwmResolution = currentMillis;

                for (byte i = 0; i < PWMS; i++)
                        pwm(i);
                recovery = false;
        }
}

unsigned int mapRound(unsigned int x, unsigned int in_min, unsigned int in_max, unsigned int out_min, unsigned int out_max)
{
        return ((x - in_min) * (out_max - out_min) + (in_max - in_min) / 2) / (in_max - in_min) + out_min;
}

unsigned int mapDoubleToInt(double x, double in_min, double in_max, int out_min, int out_max)
{
        return (unsigned int)((x - in_min) * ((double)out_max - (double)out_min)  / (in_max - in_min) + (double)out_min);
}

double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
        return (double)((x - in_min) * ((double)out_max - (double)out_min)  / (in_max - in_min) + (double)out_min);
}
