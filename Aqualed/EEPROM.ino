#include <Arduino.h>

/*
   Aqualed EEPROM functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

// kompatybilnosc z AQMA
 static int getEEPROMAddr( byte n ) {
        if (n == 0) return 280;
        if (n == 1) return 300;
        if (n == 2) return 330;
        if (n == 3) return 370;
        if (n == 4) return 390;
        if (n == 5) return 410;
        // nie aqma
        if (n == 6) return 450;
        if (n == 7) return 470;

        return 0;
}

static void writeEEPROMPWMConfig (byte pwmNumber)
{
        int startAddr = getEEPROMAddr( pwmNumber );
        EEPROM.write( startAddr + 1,  pwmChannel[pwmNumber].pin );
        EEPROM.write( startAddr + 2,  pwmChannel[pwmNumber].enabled );
        EEPROM.write( startAddr + 3,  pwmChannel[pwmNumber].onHour );
        EEPROM.write( startAddr + 4,  pwmChannel[pwmNumber].onMinute );
        EEPROM.write( startAddr + 5,  pwmChannel[pwmNumber].useLunarPhase );
        EEPROM.write( startAddr + 6,  pwmChannel[pwmNumber].offHour );
        EEPROM.write( startAddr + 7,  pwmChannel[pwmNumber].offMinute );
        EEPROM.write( startAddr + 9,  pwmChannel[pwmNumber].valueNight );
        EEPROM.write( startAddr + 10,  pwmChannel[pwmNumber].valueDay );
        EEPROM.write( startAddr + 11,  pwmChannel[pwmNumber].sunriseLenght );
        EEPROM.write( startAddr + 12,  pwmChannel[pwmNumber].sunsetLenght );
        EEPROM.write( startAddr + 13,  pwmChannel[pwmNumber].isNightLight );
        EEPROM.write( startAddr + 15,  pwmChannel[pwmNumber].valueProg );
        EEPROM.write( startAddr + 16,  pwmChannel[pwmNumber].isProg );
        EEPROM.write( startAddr + 17,  pwmChannel[pwmNumber].pin );
        EEPROM.write( startAddr + 18,  pwmChannel[pwmNumber].isI2C );
        EEPROM.write( startAddr + 19,  pwmChannel[pwmNumber].invertPwm );
        EEPROM.write( startAddr + 20,  pwmChannel[pwmNumber].watts );
}

static boolean isFirstRun ()
{
    //  return true;
        if (EEPROM.read( 100 ) != 255) return true;
        else return false;
}

static void writeEEPROMDefaults ()
{
        if (isFirstRun())
        {
                for (uint16_t i = 0; i < EEPROM.length(); i++) {
                        EEPROM.write(i, 0);
                }
                //defaults
                SETTINGS.forceNight = 0;
                SETTINGS.forceAmbient = 0;
                SETTINGS.forceOFF = 0;
                SETTINGS.max_led_temp = 35;
                SETTINGS.max_sump_temp = 35;
                SETTINGS.max_water_temp = 26;
                SETTINGS.pwmDimmingTime = 30;
                SETTINGS.screenSaverTime = 30;
                SETTINGS.softDimming = 0;
                SETTINGS.dst = 0;
                for (byte i = 0; i < 8; i++)
                {
                        SETTINGS.waterSensorAddress[i] = 0;
                        SETTINGS.ledSensorAddress[i] = 0;
                        SETTINGS.sumpSensorAddress[i] = 0;
                }

                for (byte i = 0; i < PWMS; i++ )
                {
                        writeEEPROMPWMConfig (i);
                }
                writeEEPROMSettings ();
                EEPROM.write( 100, 255 );
        }
}

static void writeEEPROMSettings ()
{
        EEPROM.write( 101, SETTINGS.forceNight);
        EEPROM.write( 102, SETTINGS.forceAmbient);
        EEPROM.write( 103, SETTINGS.forceOFF);
        EEPROM.write( 104, SETTINGS.max_led_temp);
        EEPROM.write( 105, SETTINGS.max_water_temp);
        EEPROM.write( 106, SETTINGS.pwmDimmingTime);
        EEPROM.write( 107, SETTINGS.max_sump_temp);
        EEPROM.write( 108, SETTINGS.screenSaverTime);
        EEPROM.write( 109, SETTINGS.softDimming);
        EEPROM.write( 110, SETTINGS.dst);

        // 24 bajty adresy sensoow
        byte startAddr = 120;
        for (byte i = 0; i < 8; i++)
        {
                EEPROM.write( startAddr + i, SETTINGS.ledSensorAddress[i]);
        }
        startAddr = 130;
        for (byte i = 0; i < 8; i++)
        {
                EEPROM.write( startAddr + i, SETTINGS.sumpSensorAddress[i]);
        }
        startAddr = 140;
        for (byte i = 0; i < 8; i++)
        {
                EEPROM.write( startAddr + i, SETTINGS.waterSensorAddress[i]);
        }
}

static void readEEPROMSettings ()
{
        SETTINGS.forceNight = EEPROM.read( 101 );
        SETTINGS.forceAmbient = EEPROM.read( 102);
        SETTINGS.forceOFF = EEPROM.read( 103);
        SETTINGS.max_led_temp = EEPROM.read( 104);
        SETTINGS.max_water_temp = EEPROM.read( 105);
        SETTINGS.pwmDimmingTime = EEPROM.read( 106);
        SETTINGS.max_sump_temp = EEPROM.read( 107);
        SETTINGS.screenSaverTime = EEPROM.read( 108);
        SETTINGS.softDimming = EEPROM.read( 109);
        SETTINGS.dst = EEPROM.read( 110);

        // 24 bajty adresy sensoow
        byte startAddr = 120;
        for (byte i = 0; i < 8; i++)
        {
                SETTINGS.ledSensorAddress[i] = EEPROM.read( startAddr + i);
        }
        startAddr = 130;
        for (byte i = 0; i < 8; i++)
        {
                SETTINGS.sumpSensorAddress[i] = EEPROM.read( startAddr + i);
        }
        startAddr = 140;
        for (byte i = 0; i < 8; i++)
        {
                SETTINGS.waterSensorAddress[i] = EEPROM.read( startAddr + i);
        }
}

static void eEpromRead( ) {
        for (byte i = 0; i < PWMS; i++)
        {
                int startAddr = getEEPROMAddr( i );
                pwmChannel[i].enabled = EEPROM.read( startAddr + 2 );
                pwmChannel[i].onHour = EEPROM.read( startAddr + 3 );
                pwmChannel[i].onMinute = EEPROM.read( startAddr + 4 );
                pwmChannel[i].useLunarPhase = EEPROM.read( startAddr + 5 );
                pwmChannel[i].offHour = EEPROM.read( startAddr + 6 );
                pwmChannel[i].offMinute = EEPROM.read( startAddr + 7 );
                pwmChannel[i].valueNight = EEPROM.read( startAddr + 9 );
                pwmChannel[i].valueDay = EEPROM.read( startAddr + 10 );
                pwmChannel[i].sunriseLenght = EEPROM.read( startAddr + 11 );
                pwmChannel[i].sunsetLenght = EEPROM.read( startAddr + 12 );
                pwmChannel[i].isNightLight = EEPROM.read( startAddr + 13 );
                pwmChannel[i].valueTest = EEPROM.read( startAddr + 14 );
                pwmChannel[i].valueProg = EEPROM.read( startAddr + 15 );
                pwmChannel[i].isProg = EEPROM.read( startAddr + 16 );
                pwmChannel[i].pin = EEPROM.read( startAddr + 17 );
                pwmChannel[i].isI2C = EEPROM.read( startAddr + 18 );
                pwmChannel[i].invertPwm = EEPROM.read( startAddr + 19 );
                pwmChannel[i].watts = EEPROM.read( startAddr + 20 );

        }
        readEEPROMSettings ();
}
