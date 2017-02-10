#include <Arduino.h>

#ifndef NO_I2C
    #include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm_i2c;
#endif

/*
      AquaLed - sterownik oswietlenia akwarium morskiego
       - max 6 PWM,
       - 3 czujnki termeratury,
       - 3 przekazniki na wentylatory
       - wyswietlacz Nextion
     (c) 2016 Tomek Formanowski
     Open Source public domain

     Fragmenty kodu: bluetooth ze sterownika Aqma by Magu, kombatybilnosc zachowana w zakresie obslugi przez bluetooth

 */

void setupPWMPins ()
{
        // assign pins
        pwmChannel[0].pwmPin = PWM1_PIN;
        pwmChannel[0].pwmI2C = PWM1_I2C;

        pwmChannel[1].pwmPin = PWM2_PIN;
        pwmChannel[1].pwmI2C = PWM2_I2C;

        pwmChannel[2].pwmPin = PWM3_PIN;
        pwmChannel[2].pwmI2C = PWM3_I2C;

        pwmChannel[3].pwmPin = PWM4_PIN;
        pwmChannel[3].pwmI2C = PWM4_I2C;

        pwmChannel[4].pwmPin = PWM5_PIN;
        pwmChannel[4].pwmI2C = PWM5_I2C;

        pwmChannel[5].pwmPin = PWM6_PIN;
        pwmChannel[5].pwmI2C = PWM6_I2C;

        pwmChannel[6].pwmPin = PWM7_PIN;
        pwmChannel[6].pwmI2C = PWM7_I2C;

        pwmChannel[7].pwmPin = PWM8_PIN;
        pwmChannel[7].pwmI2C = PWM8_I2C;


        // setup pins
        for (int i = 0; i < PWMS; i++)
        {
                if (pwmChannel[i].pwmI2C == 0)
                {
                        pinMode(pwmChannel[i].pwmPin, OUTPUT);
                        digitalWrite(pwmChannel[i].pwmPin, OFF);
                }
                pwmChannel[i].pwmNow = 0;
                pwmChannel[i].pwmTest = 0;
                pwmChannel[i].dimmingStart = false;
                pwmChannel[i].recoverLastState = true;

        }

        #ifndef NO_I2C
        pwm_i2c = Adafruit_PWMServoDriver();
        pwm_i2c.begin();
        pwm_i2c.setPWMFreq(I2C_FREQ);
        #endif

        //TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
        //  TCCR1A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
//        TCCR2A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);

}

// sciemnianie/rozjasnianie
boolean pwmStep (byte i, long dimmingTime)
{
        boolean dimming = false;
        double pwmNow = pwmChannel[i].pwmNow;
        byte pwmGoal = pwmChannel[i].pwmGoal;

        // mamy co chcemy, precz
        if (pwmNow == pwmGoal)
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
                //max = (double) abs (pwmChannel[i].pwmGoal - pwmChannel[i].pwmNow);

                step = (double) ( (double) max / (double) (dimmingTime / PWM_RESOLUTION));

        }


        if (step < PWM_MIN_STEP) step = PWM_MIN_STEP;
        byte stepsLeft = (pwmGoal - pwmNow) / step;
        if (stepsLeft < 0) stepsLeft *= -1;
        //long millisLeft = stepsLeft * PWM_RESOLUTION;
        //   long estTimeEnd = currentMillis+millisLeft;
        if (pwmGoal > pwmNow)
        {
                pwmNow = pwmNow + step;
                if (pwmNow >= pwmGoal || pwmNow+0.1 >=pwmGoal ) pwmNow = pwmGoal;
        }
        else
        {
                pwmNow = pwmNow - step;
                if (pwmNow <= 0 || pwmNow < 0.1) pwmNow = 0;
        }
        pwmChannel[i].pwmNow = pwmNow;
        return true;
}

// calculate when dimming actualy starts to scale dimming values
static bool isDimmingStart (int i)
{
        if (pwmChannel[i].pwmGoal != pwmChannel[i].pwmNow  && !pwmChannel[i].dimmingStart) return true;
        else return false;
}

static void forcePWMRecovery ()
{
        for (int i = 0; i < PWMS; i++)
        {
                pwmChannel[i].recoverLastState = true;
        }
}

// calculate and set pwm value and drive led
static void pwm( byte i )
{
        pwmChannel[i].isSunset= false;
        pwmChannel[i].isSunrise= false;
        pwmChannel[i].isNight= false;
        boolean dimming = false;
        long dimmingTime = (long) SETTINGS.pwmDimmingTime * (long)1000;
        long ssMillis;
        long srMillis;
        bool state = getState (i);

        //test mode
        if (testMode)
        {
                pwmChannel[i].pwmNow = pwmChannel[i].pwmTest;
                pwmChannel[i].dimmingStart = false;
        }
        else
        // force off
        if (pwmChannel[i].pwmStatus == 0 || SETTINGS.forceOFF == 1)
        {
                pwmChannel[i].pwmGoal = 0;
                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = pwmChannel[i].pwmNow;
                }

                dimming = pwmStep (i, dimmingTime);
        }
        else
        // force night
        if (SETTINGS.forceNight == 1)
        {
                if (pwmChannel[i].pwmKeepLight)
                {
                  #ifdef PWM_FORCE_NIGHT_VALUE
                        pwmChannel[i].pwmGoal = PWM_FORCE_NIGHT_VALUE;
                  #else
                        pwmChannel[i].pwmGoal = pwmChannel[i].pwmMin;
                  #endif
                }
                else
                        pwmChannel[i].pwmGoal = 0;

                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmMin);
                }
                pwmChannel[i].isNight= true;
                dimming = pwmStep (i, dimmingTime);
        }
        else
        // ambient/user program
        if (SETTINGS.forceAmbient == 1)
        {
                pwmChannel[i].pwmGoal = pwmChannel[i].pwmAmbient;
                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmAmbient);
                }

                dimming = pwmStep (i, dimmingTime);
        }
        // night light
        else if (!state && pwmChannel[i].pwmKeepLight)
        {
              #ifdef PWM_FORCE_NIGHT_VALUE
                pwmChannel[i].pwmGoal = PWM_FORCE_NIGHT_VALUE;
              #else
                pwmChannel[i].pwmGoal = pwmChannel[i].pwmMin;
              #endif

                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmMin);
                }
                pwmChannel[i].isNight= true;
                dimming = pwmStep (i, dimmingTime);
        }
        else
        //sunset
        if ( getSunsetMillis (i, ssMillis) > 0)
        {
                // scale current pwm value
                // restore state after shutdown or forced mode
                if (pwmChannel[i].recoverLastState)
                {
                        pwmChannel[i].pwmGoal = pwmChannel[i].pwmSaved;
                        if (isDimmingStart(i))
                        {
                                pwmChannel[i].dimmingStart = true;
                                pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmSaved);
                        }

                        dimming = pwmStep (i,dimmingTime);
                }
                else
                {
                        pwmChannel[i].isSunset = true;
                        if (pwmChannel[i].pwmKeepLight) pwmChannel[i].pwmGoal = pwmChannel[i].pwmMin; else pwmChannel[i].pwmGoal = 0;
                        if (isDimmingStart(i))
                        {
                                pwmChannel[i].dimmingStart = true;
                                pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmGoal);
                                pwmChannel[i].dimmingTime = ssMillis;
                        }
                }
                dimming = pwmStep (i, pwmChannel[i].dimmingTime);
        }
        else
        //sunrise
        if ( getSunriseMillis(i, srMillis) > 0)
        {
                // restore state after shutdown or forced mode
                if (pwmChannel[i].recoverLastState)
                {
                        pwmChannel[i].pwmGoal = pwmChannel[i].pwmSaved;
                        if (isDimmingStart(i))
                        {
                                pwmChannel[i].dimmingStart = true;
                                pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmSaved);
                        }

                        dimming = pwmStep (i,dimmingTime);
                }
                else
                {
                        pwmChannel[i].isSunrise = true;
                        pwmChannel[i].pwmGoal = pwmChannel[i].pwmMax;

                        if (isDimmingStart(i))
                        {
                                pwmChannel[i].dimmingStart = true;
                                pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmMax);
                                pwmChannel[i].dimmingTime = srMillis;

                        }
                }
                dimming = pwmStep (i, pwmChannel[i].dimmingTime);
        }
        else if (state)
        {
                pwmChannel[i].pwmGoal = pwmChannel[i].pwmMax;

                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = abs(pwmChannel[i].pwmNow-pwmChannel[i].pwmMax);
                }

                dimming = pwmStep (i,dimmingTime);
        }
        // scheduled off
        else if (!state && !pwmChannel[i].pwmKeepLight)
        {
                pwmChannel[i].pwmGoal = 0;

                if (isDimmingStart(i))
                {
                        pwmChannel[i].dimmingStart = true;
                        pwmChannel[i].dimmingScale = pwmChannel[i].pwmNow;
                }
                dimming = pwmStep (i,dimmingTime);
        }

        // no change
        if (pwmLast[i] == pwmChannel[i].pwmNow) return;

        if (pwmChannel[i].pwmI2C == 0)
        {
                byte val = (byte) pwmChannel[i].pwmNow;
                // logarithmic dimming table, experimental, works best if max 100%
                if (dimming && SETTINGS.softDimming == 1 && (byte) val != pwmChannel[i].pwmGoal)
                {

                        val = (byte)pgm_read_byte(&dimmingTable[val]);
                        //val = dimmingTable[val];
                }
                analogWrite( pwmChannel[i].pwmPin,  val);
        }
        else
        {
          #ifndef NO_I2C
                // pwm module does not need constant updates
                if (pwmLast[i] != pwmChannel[i].pwmNow)
                {
                        long v;
                        if (i2c_invert == 1)
                                v = mapDouble(pwmChannel[i].pwmNow, 255.0, 0.0, PWM_I2C_MIN, PWM_I2C_MAX);
                        else
                                v = mapDouble(pwmChannel[i].pwmNow, 0.0, 255.0, PWM_I2C_MIN, PWM_I2C_MAX);
                        pwm_i2c.setPWM(pwmChannel[i].pwmPin, 0, v );
                }
          #endif

        }
        pwmLast[i] = pwmChannel[i].pwmNow;
}

// check if scheduled on or off
bool getState (byte i)
{
        boolean midnight = false;
        long pwmOn = (long(pwmChannel[i].pwmHOn) * 3600) + (long(pwmChannel[i].pwmMOn) * 60);
        long pwmOff = (long(pwmChannel[i].pwmHOff) * 3600) + (long(pwmChannel[i].pwmMOff) * 60);
        long currTime = (long)tm.Hour * 60 * 60 + (long)tm.Minute * 60 + (long)tm.Second;

        if (pwmOff < pwmOn) midnight = true;

        if (!midnight)
        {
                if (currTime>=pwmOn && currTime < pwmOff) return true;
        }
        else
        {
                if (currTime>=pwmOn && currTime < 86400) return true;
                else if (currTime>=0 && currTime < pwmOff) return true;
        }

        return false;
}

// calculate remaining sunset time (if any)
long getSunsetMillis (byte i, long &m)
{
        boolean midnight = false;
        long stopTime = (long)pwmChannel[i].pwmHOff * 60 * 60 + (long)pwmChannel[i].pwmMOff * 60;
        long currTime = (long)tm.Hour * 60 * 60 + (long)tm.Minute * 60 + (long)tm.Second;
        long startTime = stopTime - (long) pwmChannel[i].pwmSs * 60;

        if (startTime < 0) midnight = true;

        // before midnight
        if (currTime >= startTime && currTime <= stopTime)
        {
                m = (long) (stopTime - currTime) * 1000;
                return m;
        }

        // midnight crossing
        if (midnight)
        {
                stopTime += 86400;
                long startTime = stopTime - (long) pwmChannel[i].pwmSs * 60;

                if (currTime >= startTime && currTime <= stopTime)
                {
                        m = (long) (stopTime - currTime) * 1000;
                        return m;
                }
        }

        m = 0;
        return m;

}

// calculate remaining sunrise time (if any)
long getSunriseMillis (byte i, long &m)
{
        // in seconds
        long startTime = (long)pwmChannel[i].pwmHOn * 60 * 60 + (long)pwmChannel[i].pwmMOn * 60;
        long currTime = (long)tm.Hour * 60 * 60 + (long)tm.Minute * 60 + (int)tm.Second;
        long stopTime = startTime + (long)pwmChannel[i].pwmSr * 60;

        boolean midnight = false;

        // check if midnight cross
        if (stopTime > 86400) midnight = true;

        // before midnight
        if (currTime >= startTime && currTime <= stopTime)
        {
                m = (long) (stopTime - currTime) * 1000;
                return m;
        }

        // if crossing midnight
        if (midnight)
        {
                currTime += 86400;
                if (currTime >= startTime && currTime <= stopTime)
                {
                        m = (long) (stopTime - currTime) * 1000;
                        return m;
                }
        }
        m = 0;
        return m;
}

// main pwm loop
void pwm ()
{
        if (currentMillis - previousPwmResolution > PWM_RESOLUTION)
        {
                previousPwmResolution = currentMillis;
                for (int i = 0; i < PWMS; i++)
                {
                        pwm(i);

                }
        }
        if (currentMillis - previousMillisEepromState > EEPROM_STATE_RESOLUTION)
        {
                previousMillisEepromState = currentMillis;
                for (int i = 0; i < PWMS; i++)
                {
                        // write state only in sunset/sunrise operation
                        if (pwmChannel[i].isSunset || pwmChannel[i].isSunrise)
                        {
                                pwmChannel[i].pwmSaved = pwmChannel[i].pwmNow;
                                writeEEPROMPWMState(i);
                        }
                }
        }
}

long mapRound(long x, long in_min, long in_max, long out_min, long out_max)
{
        return ((x - in_min) * (out_max - out_min) + (in_max - in_min) / 2) / (in_max - in_min) + out_min;
}

int mapDouble(double x, double in_min, double in_max, int out_min, int out_max)
{
        return (int)((x - in_min) * ((double)out_max - (double)out_min)  / (in_max - in_min) + (double)out_min);
}
