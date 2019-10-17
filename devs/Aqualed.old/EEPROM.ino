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
        EEPROM.write( startAddr + 1,  pwmSettings[pwmNumber].pin );
        EEPROM.write( startAddr + 2,  pwmSettings[pwmNumber].enabled );
        EEPROM.write( startAddr + 3,  pwmSettings[pwmNumber].onHour );
        EEPROM.write( startAddr + 4,  pwmSettings[pwmNumber].onMinute );
        EEPROM.write( startAddr + 5,  pwmSettings[pwmNumber].useLunarPhase );
        EEPROM.write( startAddr + 6,  pwmSettings[pwmNumber].offHour );
        EEPROM.write( startAddr + 7,  pwmSettings[pwmNumber].offMinute );
        EEPROM.write( startAddr + 8,  pwmSettings[pwmNumber].valueNight );
        EEPROM.write( startAddr + 9,  highByte (pwmSettings[pwmNumber].valueDay ));
        EEPROM.write( startAddr + 10, lowByte (pwmSettings[pwmNumber].valueDay ));
        EEPROM.write( startAddr + 11, pwmSettings[pwmNumber].sunriseLenght );
        EEPROM.write( startAddr + 12, pwmSettings[pwmNumber].sunsetLenght );
        EEPROM.write( startAddr + 13, pwmSettings[pwmNumber].isNightLight );
        EEPROM.write( startAddr + 14, highByte(pwmSettings[pwmNumber].valueProg ));
        EEPROM.write( startAddr + 15, lowByte(pwmSettings[pwmNumber].valueProg ));
        EEPROM.write( startAddr + 16, pwmSettings[pwmNumber].isProg );
        EEPROM.write( startAddr + 17, pwmSettings[pwmNumber].pin );
        EEPROM.write( startAddr + 18, pwmSettings[pwmNumber].isI2C );
        EEPROM.write( startAddr + 19, pwmSettings[pwmNumber].invertPwm );
        EEPROM.write( startAddr + 20, pwmSettings[pwmNumber].watts );
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
                memset (&settings, 0, sizeof(settings));

                for (byte i = 0; i < PWMS; i++ )
                {
                        memset (&pwmSettings[i], 0, sizeof(pwmSettings[i]));
                        writeEEPROMPWMConfig (i);
                }

                writeEEPROMSettings ();
                EEPROM.write( 100, 255 );
        }
}

static void writeEEPROMForceNight ()
{
        EEPROM.write( 101, settings.forceNight);
}

static void writeEEPROMForceOff ()
{
        EEPROM.write( 101, settings.forceOFF);
}

static void writeEEPROMForceAmbient ()
{
        EEPROM.write( 101, settings.forceAmbient);
}

static void writeEEPROMSensor (byte startAddr, byte addr[])
{
        for (byte i = 0; i < 8; i++)
        {
                EEPROM.write( startAddr + i, addr[i]);
        }
}
static void readEEPROMSensor (byte startAddr, byte addr[])
{
        for (byte i = 0; i < 8; i++)
        {
                addr[i] = EEPROM.read( startAddr + i);
        }

}

static void writeEEPROMSensors ()
{
        writeEEPROMSensor (120,  settings.ledSensorAddress);
        writeEEPROMSensor (130,  settings.sumpSensorAddress);
        writeEEPROMSensor (140,  settings.waterSensorAddress);
}

static void writeEEPROMSettings ()
{
        EEPROM.write( 104, settings.max_led_temp);
        EEPROM.write( 105, settings.max_water_temp);
        EEPROM.write( 106, settings.pwmDimmingTime);
        EEPROM.write( 107, settings.max_sump_temp);
        EEPROM.write( 108, settings.screenSaverTime);
        EEPROM.write( 109, settings.softDimming);
        EEPROM.write( 110, settings.dst);
}

static void readEEPROMSettings ()
{
        settings.forceNight = EEPROM.read( 101 );
        settings.forceAmbient = EEPROM.read( 102);
        settings.forceOFF = EEPROM.read( 103);
        settings.max_led_temp = EEPROM.read( 104);
        settings.max_water_temp = EEPROM.read( 105);
        settings.pwmDimmingTime = EEPROM.read( 106);
        settings.max_sump_temp = EEPROM.read( 107);
        settings.screenSaverTime = EEPROM.read( 108);
        settings.softDimming = EEPROM.read( 109);
        settings.dst = EEPROM.read( 110);

        // 24 bajty adresy sensoow
        readEEPROMSensor (120,  settings.ledSensorAddress);
  readEEPROMSensor (130, settings.sumpSensorAddress);
  readEEPROMSensor (140, settings.waterSensorAddress);


}

static void eEpromRead( ) {
        for (byte i = 0; i < PWMS; i++)
        {
                int startAddr = getEEPROMAddr( i );
                pwmSettings[i].enabled = EEPROM.read( startAddr + 2 );
                pwmSettings[i].onHour = EEPROM.read( startAddr + 3 );
                pwmSettings[i].onMinute = EEPROM.read( startAddr + 4 );
                pwmSettings[i].useLunarPhase = EEPROM.read( startAddr + 5 );
                pwmSettings[i].offHour = EEPROM.read( startAddr + 6 );
                pwmSettings[i].offMinute = EEPROM.read( startAddr + 7 );
                pwmSettings[i].valueNight = EEPROM.read( startAddr + 8 );
                pwmSettings[i].valueDay = word(EEPROM.read( startAddr + 9 ),EEPROM.read( startAddr + 10) );
                pwmSettings[i].sunriseLenght = EEPROM.read( startAddr + 11 );
                pwmSettings[i].sunsetLenght = EEPROM.read( startAddr + 12 );
                pwmSettings[i].isNightLight = EEPROM.read( startAddr + 13 );
                pwmSettings[i].valueProg = word(EEPROM.read( startAddr + 14 ),EEPROM.read( startAddr + 15) );
                pwmSettings[i].isProg = EEPROM.read( startAddr + 16 );
                pwmSettings[i].pin = EEPROM.read( startAddr + 17 );
                pwmSettings[i].isI2C = EEPROM.read( startAddr + 18 );
                pwmSettings[i].invertPwm = EEPROM.read( startAddr + 19 );
                pwmSettings[i].watts = EEPROM.read( startAddr + 20 );
        }
        readEEPROMSettings ();
}
