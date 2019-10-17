#include <Arduino.h>

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
        // I2C = 0 - piny PWM Arduino
        // I2C = 1 - kontroler i2c PWM np. PCA9685

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

        //TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
        TCCR1A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
        TCCR2A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);

}

// sciemnianie/rozjasnianie
boolean pwmStep (byte i, long silkyTime)
{
        boolean dimming = false;
        double pwmNow = pwm_list[i].pwmNow;
        int pwmGoal = pwm_list[i].pwmGoal;

        // mamy co chcemy, precz
        if (pwmNow == pwmGoal) return dimming;

        double step;
        step = (double) (double)255 / (double)(silkyTime / PWM_RESOLUTION);
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

// glowna petla pwm
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

        //test
        if (testMode)
        {
                pwm_list[i].pwmNow = pwm_list[i].pwmTest;
        }
        else
        // off
        if (pwm_list[i].pwmStatus == 0 || SETTINGS.forceOFF == 1)
        {
                pwm_list[i].pwmGoal = 0;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        else
        // night
        if (SETTINGS.forceNight == 1)
        {
                if (pwm_list[i].pwmKeepLight) pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
                else
                        pwm_list[i].pwmGoal = 0;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        else
        // ambient
        if (SETTINGS.forceAmbient == 1)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmAmbient;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }
        else if (!state && pwm_list[i].pwmKeepLight)
        {
                pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
                dimming = pwmStep (i, SETTINGS.pwmDimmingTime * 1000);
        }

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
        byte val = (byte) pwm_list[i].pwmNow;

        // table
        if (dimming && SETTINGS.softDimming == 1 && (byte) val != pwm_list[i].pwmGoal)
        {
                //  val = (byte)pgm_read_byte(&dimmingTable[val]);
                val = dimmingTable[val];
        }
        if (pwm_list[i].pwmI2C == 0)
        {
                analogWrite( pwm_list[i].pwmPin,  val);
        }
        else
        {
                // tu można by się pokusić o wykorzystanie 12bitowej rozdzielczości sterownika - na razie jest mapowanie z 8bit na 12bit
                long v = mapRound(val, 0, 255, PWM_I2C_MIN, PWM_I2C_MAX);
                pwm_i2c.setPWM(pwm_list[i].pwmPin, 0, v );
        }

}



long getSunsetMillis (byte i, long &m)
{
        boolean midnight = false;
        int stopTime = (int)pwm_list[i].pwmHOff * 60 * 60 + (int)pwm_list[i].pwmMOff * 60 - (int)pwm_list[i].pwmSOff;
        int currTime = (int)tm.Hour * 60 * 60 + (int)tm.Minute * 60 + (int)tm.Second;
        int startTime = stopTime - (int) pwm_list[i].pwmSs * 60;

        if (startTime < 0) midnight = true;

        // przed północą
        if (currTime >= startTime && currTime <= stopTime)
        {
                m = (long) (stopTime - currTime) * 1000;
                return m;
        }

        // przechodzi przez północ
        if (midnight)
        {
                stopTime += 86400;
                int startTime = stopTime - (int) pwm_list[i].pwmSs * 60;

                if (currTime >= startTime && currTime <= stopTime)
                {
                        m = (long) (stopTime - currTime) * 1000;
                        return m;
                }
        }

        m = 0;
        return m;

}

long getSunriseMillis (byte i, long &m)
{
        // w sekundach
        int startTime = (int)pwm_list[i].pwmHOn * 60 * 60 + (int)pwm_list[i].pwmMOn * 60 + (int)pwm_list[i].pwmSOn;
        int currTime = (int)tm.Hour * 60 * 60 + (int)tm.Minute * 60 + (int)tm.Second;
        int stopTime = startTime + (int)pwm_list[i].pwmSr * 60;

        boolean midnight = false;

        // sprawdz czy przejście przez północ
        if (stopTime > 86400) midnight = true;

        // przed północą - standardowo
        if (currTime >= startTime && currTime <= stopTime)
        {
                m = (long) (stopTime - currTime) * 1000;
                return m;
        }

        // jesli polnoc
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
