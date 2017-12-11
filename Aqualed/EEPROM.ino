#include <Arduino.h>

/*
   Aqualed EEPROM functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
        const byte* p = (const byte*)(const void*)&value;
        unsigned int i;
        for (i = 0; i < sizeof(value); i++)
                EEPROM.write(ee++, *p++);
        return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
        byte* p = (byte*)(void*)&value;
        unsigned int i;
        for (i = 0; i < sizeof(value); i++)
                *p++ = EEPROM.read(ee++);
        return i;
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
        EEPROM.write( 103, settings.forceOFF);
}

static void writeEEPROMForceAmbient ()
{
        EEPROM.write( 102, settings.forceAmbient);
}

static void writeEEPROMSettings ()
{
        EEPROM_writeAnything(101, settings);
}

static void readEEPROMSettings ()
{
        EEPROM_readAnything (101, settings);
}

static void eEpromRead( ) {
        for (byte i = 0; i < PWMS; i++)
        {
                EEPROM_readAnything (getEEPROMAddr( i )+2, pwmSettings[i]);
        }
        readEEPROMSettings ();
}

static void writeEEPROMPWMConfig (byte pwmNumber)
{
        EEPROM_writeAnything (getEEPROMAddr( pwmNumber )+2, pwmSettings[pwmNumber]);

}

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
