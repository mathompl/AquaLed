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
        pwm_list[0].pwmPin = PWM1_PIN;
        pwm_list[0].pwmI2C = PWM1_I2C;

        pwm_list[1].pwmPin = PWM2_PIN;
        pwm_list[1].pwmI2C = PWM2_I2C;

        pwm_list[2].pwmPin = PWM3_PIN;
        pwm_list[2].pwmI2C = PWM3_I2C;

        pwm_list[3].pwmPin = PWM4_PIN;
        pwm_list[3].pwmI2C = PWM4_I2C;

        pwm_list[4].pwmPin = PWM5_PIN;
        pwm_list[4].pwmI2C = PWM5_I2C;

        pwm_list[5].pwmPin = PWM6_PIN;
        pwm_list[5].pwmI2C = PWM6_I2C;

        pwm_list[6].pwmPin = PWM7_PIN;
        pwm_list[6].pwmI2C = PWM7_I2C;

        pwm_list[7].pwmPin = PWM8_PIN;
        pwm_list[7].pwmI2C = PWM8_I2C;


        // setup pins
        for (int i = 0; i < PWMS; i++)
        {
                if (pwm_list[i].pwmI2C == 0)
                {
                        pinMode(pwm_list[i].pwmPin, OUTPUT);
                        digitalWrite(pwm_list[i].pwmPin, OFF);
                }
                pwm_list[i].pwmNow = 0;
                pwm_list[i].pwmTest = 0;
                pwm_list[i].dimmingStart = false;
                pwm_list[i].recoverLastState = true;

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
        double pwmNow = pwm_list[i].pwmNow;
        byte pwmGoal = pwm_list[i].pwmGoal;

        // mamy co chcemy, precz
        if (pwmNow == pwmGoal)
        {
                pwm_list[i].dimmingStart = false;
                pwm_list[i].recoverLastState = false;
                return dimming;
        }

        double step;

// full pwm scale dimming when no other data
        double max = 255;

// scale dimming
        if (pwm_list[i].dimmingStart == true)
        {
                max = (double) pwm_list[i].dimmingScale;
                //max = (double) abs (pwm_list[i].pwmGoal - pwm_list[i].pwmNow);

                step = (double) ( (double) max / (double) (dimmingTime / PWM_RESOLUTION));

        }

        #ifdef DEBUG
        if (i == 3 )
        {
                Serial.print ("step = ");
                Serial.print (step,10);
                Serial.print (" pwmnow = ");
                Serial.print (pwm_list[i].pwmNow);
                Serial.print (" pwmgoal = ");
                Serial.print (pwm_list[i].pwmGoal);
                Serial.print (" i2c = ");

                int v = mapDouble(pwm_list[i].pwmNow, 0.0, 255.0, PWM_I2C_MIN, PWM_I2C_MAX);
                Serial.print (v);
                Serial.println ();
        }
        #endif
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
        pwm_list[i].pwmNow = pwmNow;
        return true;
}

// calculate when dimming actualy starts to scale dimming values
static bool isDimmingStart (int i)
{
        if (pwm_list[i].pwmGoal != pwm_list[i].pwmNow  && !pwm_list[i].dimmingStart) return true;
        else return false;
}

static void forcePWMRecovery ()
{
        for (int i = 0; i < PWMS; i++)
        {
                pwm_list[i].recoverLastState = true;
        }
}

// calculate and set pwm value and drive led
static void pwm( byte i )
{
        pwm_list[i].isSunset= false;
        pwm_list[i].isSunrise= false;
        pwm_list[i].isNight= false;

        long ssMillis;
        long srMillis;
        boolean state = false;
        boolean dimming = false;
        long unsigned pwmOn = (long(pwm_list[i].pwmHOn) * 3600) + (long(pwm_list[i].pwmMOn) * 60) + long(pwm_list[i].pwmSOn);
        long unsigned pwmOff = (long(pwm_list[i].pwmHOff) * 3600) + (long(pwm_list[i].pwmMOff) * 60) + long(pwm_list[i].pwmSOff);

        if (pwmOn < pwmOff) if (currentTimeSec >= pwmOn && currentTimeSec < pwmOff) state = true;

        if (pwmOn > pwmOff)
        {
                if (currentTimeSec >= pwmOn && currentTimeSec <= 86400) state = true;
                if (currentTimeSec >= 0 && currentTimeSec < pwmOff) state = true;
        }

        long dimmingTime = (long) SETTINGS.pwmDimmingTime * (long)1000;
        //test mode
        if (testMode)
        {
                pwm_list[i].pwmNow = pwm_list[i].pwmTest;
                pwm_list[i].dimmingStart = false;
        }
        else
        // force off
        if (pwm_list[i].pwmStatus == 0 || SETTINGS.forceOFF == 1)
        {
                pwm_list[i].pwmGoal = 0;
                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = pwm_list[i].pwmNow;
                }

                dimming = pwmStep (i, dimmingTime);
        }
        else
        // force night
        if (SETTINGS.forceNight == 1)
        {
                if (pwm_list[i].pwmKeepLight)
                {
                  #ifdef PWM_FORCE_NIGHT_VALUE
                        pwm_list[i].pwmGoal = PWM_FORCE_NIGHT_VALUE;
                  #else
                        pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
                  #endif
                }
                else
                        pwm_list[i].pwmGoal = 0;

                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = abs(pwm_list[i].pwmNow-pwm_list[i].pwmMin);
                }
                pwm_list[i].isNight= true;
                dimming = pwmStep (i, dimmingTime);
        }
        else
        // ambient/user program
        if (SETTINGS.forceAmbient == 1)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmAmbient;
                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = abs(pwm_list[i].pwmNow-pwm_list[i].pwmAmbient);
                }

                dimming = pwmStep (i, dimmingTime);
        }
        // restore state after shutdown or forced mode
        else if (pwm_list[i].recoverLastState)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmSaved;
                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = abs(pwm_list[i].pwmNow-pwm_list[i].pwmSaved);
                }

                dimming = pwmStep (i,dimmingTime);
        }
        // night light
        else if (!state && pwm_list[i].pwmKeepLight)
        {
              #ifdef PWM_FORCE_NIGHT_VALUE
                pwm_list[i].pwmGoal = PWM_FORCE_NIGHT_VALUE;
              #else
                pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
              #endif

                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = abs(pwm_list[i].pwmNow-pwm_list[i].pwmMin);
                }
                pwm_list[i].isNight= true;
                dimming = pwmStep (i, dimmingTime);
        }

        else
        //sunset
        if ( getSunsetMillis (i, ssMillis) > 0)
        {
                // scale current pwm value

                pwm_list[i].isSunset = true;
                if (pwm_list[i].pwmKeepLight) pwm_list[i].pwmGoal = pwm_list[i].pwmMin; else pwm_list[i].pwmGoal = 0;
                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = abs(pwm_list[i].pwmNow-pwm_list[i].pwmGoal);
                        pwm_list[i].dimmingTime = ssMillis;
                }

                dimming = pwmStep (i, pwm_list[i].dimmingTime);
        }
        else
        //sunrise
        if ( getSunriseMillis(i, srMillis) > 0)
        {

                pwm_list[i].isSunrise = true;
                pwm_list[i].pwmGoal = pwm_list[i].pwmMax;

                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = abs(pwm_list[i].pwmNow-pwm_list[i].pwmMax);
                        pwm_list[i].dimmingTime = srMillis;

                }

                dimming = pwmStep (i, pwm_list[i].dimmingTime);
        }
        else if (state)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmMax;

                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = abs(pwm_list[i].pwmNow-pwm_list[i].pwmMax);
                }

                dimming = pwmStep (i,dimmingTime);
        }
        // scheduled off
        else if (!state && !pwm_list[i].pwmKeepLight)
        {
                pwm_list[i].pwmGoal = 0;

                if (isDimmingStart(i))
                {
                        pwm_list[i].dimmingStart = true;
                        pwm_list[i].dimmingScale = pwm_list[i].pwmNow;
                }
                dimming = pwmStep (i,dimmingTime);
        }

        // no change
        if (pwmLast[i] == pwm_list[i].pwmNow) return;

        if (pwm_list[i].pwmI2C == 0)
        {
                byte val = (byte) pwm_list[i].pwmNow;
                // logarithmic dimming table, experimental, works best if max 100%
                if (dimming && SETTINGS.softDimming == 1 && (byte) val != pwm_list[i].pwmGoal)
                {

                        val = (byte)pgm_read_byte(&dimmingTable[val]);
                        //val = dimmingTable[val];
                }
                analogWrite( pwm_list[i].pwmPin,  val);
        }
        else
        {
          #ifndef NO_I2C
                // pwm module does not need constant updates
                if (pwmLast[i] != pwm_list[i].pwmNow)
                {
                        long v;
                        if (i2c_invert == 1)
                                v = mapDouble(pwm_list[i].pwmNow, 255.0, 0.0, PWM_I2C_MIN, PWM_I2C_MAX);
                        else
                                v = mapDouble(pwm_list[i].pwmNow, 0.0, 255.0, PWM_I2C_MIN, PWM_I2C_MAX);
                        pwm_i2c.setPWM(pwm_list[i].pwmPin, 0, v );
                }
          #endif

        }
        pwmLast[i] = pwm_list[i].pwmNow;
}


// calculate remaining sunset time (if any)
long getSunsetMillis (byte i, long &m)
{
        boolean midnight = false;
        long stopTime = (long)pwm_list[i].pwmHOff * 60 * 60 + (long)pwm_list[i].pwmMOff * 60;
        long currTime = (long)tm.Hour * 60 * 60 + (long)tm.Minute * 60 + (long)tm.Second;
        long startTime = stopTime - (long) pwm_list[i].pwmSs * 60;

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
                long startTime = stopTime - (long) pwm_list[i].pwmSs * 60;

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
        long startTime = (long)pwm_list[i].pwmHOn * 60 * 60 + (long)pwm_list[i].pwmMOn * 60;
        long currTime = (long)tm.Hour * 60 * 60 + (long)tm.Minute * 60 + (int)tm.Second;
        long stopTime = startTime + (long)pwm_list[i].pwmSr * 60;

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
                        // write state only in normal operation
                        if (!testMode && !SETTINGS.forceOFF && !SETTINGS.forceNight && !SETTINGS.forceAmbient)
                        {
                                pwm_list[i].pwmSaved = pwm_list[i].pwmNow;
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
