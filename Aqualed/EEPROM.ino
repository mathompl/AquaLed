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



void writeEEPROMPWMConfig (byte pwmNumber)
{
        int startAddr = getEEPROMAddr( pwmNumber );
        EEPROM.write( startAddr + 1,  pwmChannel[pwmNumber].pwmPin );
        EEPROM.write( startAddr + 2,  pwmChannel[pwmNumber].pwmStatus );
        EEPROM.write( startAddr + 3,  pwmChannel[pwmNumber].pwmHOn );
        EEPROM.write( startAddr + 4,  pwmChannel[pwmNumber].pwmMOn );
        EEPROM.write( startAddr + 5,  pwmChannel[pwmNumber].pwmSOn );
        EEPROM.write( startAddr + 6,  pwmChannel[pwmNumber].pwmHOff );
        EEPROM.write( startAddr + 7,  pwmChannel[pwmNumber].pwmMOff );
        EEPROM.write( startAddr + 8,  pwmChannel[pwmNumber].pwmSOff );
        EEPROM.write( startAddr + 9,  pwmChannel[pwmNumber].pwmMin );
        EEPROM.write( startAddr + 10,  pwmChannel[pwmNumber].pwmMax );
        EEPROM.write( startAddr + 11,  pwmChannel[pwmNumber].pwmSr );
        EEPROM.write( startAddr + 12,  pwmChannel[pwmNumber].pwmSs );
        EEPROM.write( startAddr + 13,  pwmChannel[pwmNumber].pwmKeepLight );
        EEPROM.write( startAddr + 15,  pwmChannel[pwmNumber].pwmAmbient );
}

void writeEEPROMPWMState (byte pwmNumber)
{
        int startAddr = getEEPROMAddr( pwmNumber );
        EEPROM.write( startAddr + 14,  pwmChannel[pwmNumber].pwmTest );
        EEPROM.write( startAddr + 16,  (byte) pwmChannel[pwmNumber].pwmNow );
}

static boolean isFirstRun ()
{

        if (EEPROM.read( 100 ) != 255) return true;
        else return false;
}

void writeEEPROMDefaults ()
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
                for (int i = 0; i < 8; i++)
                {
                        SETTINGS.waterSensorAddress[i] = 0;
                }
                for (int i = 0; i < 8; i++)
                {
                        SETTINGS.ledSensorAddress[i] = 0;
                }
                for (int i = 0; i < 8; i++)
                {
                        SETTINGS.sumpSensorAddress[i] = 0;
                }

                for (int i = 0; i < PWMS; i++ )
                {
                        pwmChannel[i].pwmStatus = 0;
                        pwmChannel[i].pwmHOn = 0;
                        pwmChannel[i].pwmMOn = 0;
                        pwmChannel[i].pwmSOn = 0;
                        pwmChannel[i].pwmHOff = 0;
                        pwmChannel[i].pwmMOff = 0;
                        pwmChannel[i].pwmSOff = 0;
                        pwmChannel[i].pwmMin = 0;
                        pwmChannel[i].pwmMax = 0;
                        pwmChannel[i].pwmSr = 0;
                        pwmChannel[i].pwmSs = 0;
                        pwmChannel[i].pwmKeepLight = 0;
                        pwmChannel[i].pwmInvert = 0;
                        pwmChannel[i].pwmNow = 0;
                        pwmChannel[i].pwmGoal = 0;
                        pwmChannel[i].pwmSaved = 0;
                        pwmChannel[i].pwmTest = 0;
                        pwmChannel[i].isSunrise = 0;
                        pwmChannel[i].isSunset = 0;
                        pwmChannel[i].pwmAmbient = 0;

                        writeEEPROMPWMConfig (i);
                        writeEEPROMPWMState (i);
                }
                writeEEPROMSettings ();
                EEPROM.write( 100, 255 );

        }
}

void writeEEPROMSettings ()
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
        for (int i = 0; i < 8; i++)
        {
                EEPROM.write( startAddr + i, SETTINGS.ledSensorAddress[i]);
        }
        startAddr = 130;
        for (int i = 0; i < 8; i++)
        {
                EEPROM.write( startAddr + i, SETTINGS.sumpSensorAddress[i]);
        }
        startAddr = 140;
        for (int i = 0; i < 8; i++)
        {
                EEPROM.write( startAddr + i, SETTINGS.waterSensorAddress[i]);
        }

}

void readEEPROMSettings ()
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
        for (int i = 0; i < 8; i++)
        {
                SETTINGS.ledSensorAddress[i] = EEPROM.read( startAddr + i);
        }
        startAddr = 130;
        for (int i = 0; i < 8; i++)
        {
                SETTINGS.sumpSensorAddress[i] = EEPROM.read( startAddr + i);
        }
        startAddr = 140;
        for (int i = 0; i < 8; i++)
        {
                SETTINGS.waterSensorAddress[i] = EEPROM.read( startAddr + i);
        }
}



void eEpromRead( ) {
        for (int i = 0; i < PWMS; i++)
        {
                int startAddr = getEEPROMAddr( i );
                pwmChannel[i].pwmStatus = EEPROM.read( startAddr + 2 );
                pwmChannel[i].pwmHOn = EEPROM.read( startAddr + 3 );
                pwmChannel[i].pwmMOn = EEPROM.read( startAddr + 4 );
                pwmChannel[i].pwmSOn = EEPROM.read( startAddr + 5 );
                pwmChannel[i].pwmHOff = EEPROM.read( startAddr + 6 );
                pwmChannel[i].pwmMOff = EEPROM.read( startAddr + 7 );
                pwmChannel[i].pwmSOff = EEPROM.read( startAddr + 8 );
                pwmChannel[i].pwmMin = EEPROM.read( startAddr + 9 );
                pwmChannel[i].pwmMax = EEPROM.read( startAddr + 10 );
                pwmChannel[i].pwmSr = EEPROM.read( startAddr + 11 );
                pwmChannel[i].pwmSs = EEPROM.read( startAddr + 12 );
                pwmChannel[i].pwmKeepLight = EEPROM.read( startAddr + 13 );
                pwmChannel[i].pwmTest = EEPROM.read( startAddr + 14 );
                pwmChannel[i].pwmAmbient = EEPROM.read( startAddr + 15 );
                pwmChannel[i].pwmSaved = EEPROM.read( startAddr + 16);

        }
        readEEPROMSettings ();

}
