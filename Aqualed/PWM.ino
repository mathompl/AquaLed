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
        if (pwmNow == pwmGoal) return dimming;

        double step;
        step = (double) (double)255 / (double)(dimmingTime / PWM_RESOLUTION);
        if (step < PWM_MIN_STEP) step = PWM_MIN_STEP;
        byte stepsLeft = (pwmGoal - pwmNow) / step;
        if (stepsLeft < 0) stepsLeft *= -1;
        //long millisLeft = stepsLeft * PWM_RESOLUTION;
        //   long estTimeEnd = currentMillis+millisLeft;
        if (pwmGoal > pwmNow)
        {
                pwmNow = pwmNow + step;
                if (pwmNow > pwmGoal ) pwmNow = pwmGoal;
        }
        else
        {
                pwmNow = pwmNow - step;
                if (pwmNow < 0 || pwmNow < 0.1) pwmNow = 0;
        }
        pwm_list[i].pwmNow = pwmNow;
        return true;
}

// calculate and set pwm value and drive led
static void pwm( byte i )
{
        pwm_list[i].isSunrise = false;
        pwm_list[i].isSunset = false;
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

        //test mode
        if (testMode)
        {
                pwm_list[i].pwmNow = pwm_list[i].pwmTest;
        }
        else
        // force off
        if (pwm_list[i].pwmStatus == 0 || SETTINGS.forceOFF == 1)
        {
                pwm_list[i].pwmGoal = 0;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        else
        // force night
        if (SETTINGS.forceNight == 1)
        {
                if (pwm_list[i].pwmKeepLight) pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
                else
                        pwm_list[i].pwmGoal = 0;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        else
        // ambient/user program
        if (SETTINGS.forceAmbient == 1)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmAmbient;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        // night light
        else if (!state && pwm_list[i].pwmKeepLight)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        // scheduled off
        else if (!state && !pwm_list[i].pwmKeepLight)
        {
                pwm_list[i].pwmGoal = 0;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        else
        //sunset
        if ( getSunsetMillis(i, ssMillis) > 0)
        {
                pwm_list[i].isSunset = true;
                pwm_list[i].pwmGoal = 0;
                dimming = pwmStep (i, ssMillis);
        }
        else
        //sunrise
        if ( getSunriseMillis(i, srMillis) > 0)
        {
                pwm_list[i].isSunrise = true;
                pwm_list[i].pwmGoal = pwm_list[i].pwmMax;
                dimming = pwmStep (i, srMillis);
        }
        else if (state)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmMax;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }

        // no change
        if (pwmLast[i] == pwm_list[i].pwmNow) return;

        if (pwm_list[i].pwmI2C == 0)
        {
                byte val = (byte) pwm_list[i].pwmNow;
                // logarithmic dimming table, experimental, works best if max 100%
                if (dimming && SETTINGS.softDimming == 1 && (byte) val != pwm_list[i].pwmGoal)
                {

                        val = dimmingTable[val];
                }
                analogWrite( pwm_list[i].pwmPin,  val);
        }
        else
        {
          #ifndef NO_I2C
                // check - todo: 12bit resolution - now cast 8 bit to 12 bit - precision loss
                // pwm module does not need constant updates
                if (pwmLast[i] != pwm_list[i].pwmNow)
                {
                        long v = mapRound(pwm_list[i].pwmNow, 0, 255, PWM_I2C_MIN, PWM_I2C_MAX);
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
        long stopTime = (long)pwm_list[i].pwmHOff * 60 * 60 + (long)pwm_list[i].pwmMOff * 60 - (long)pwm_list[i].pwmSOff;
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
        long startTime = (long)pwm_list[i].pwmHOn * 60 * 60 + (long)pwm_list[i].pwmMOn * 60 + (long)pwm_list[i].pwmSOn;
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
                        writeEEPROMPWMState(i);

                }
        }
}

long mapRound(long x, long in_min, long in_max, long out_min, long out_max)
{
        return ((x - in_min) * (out_max - out_min) + (in_max - in_min) / 2) / (in_max - in_min) + out_min;
}
