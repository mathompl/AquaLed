#include "datastorage.h"
#include "defaults.h"
/*
   Aqualed EEPROM functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */
DataStorage::DataStorage ()
{

}
void DataStorage::begin ()
{
        if (isFirstRun())
        {
                writeEEPROMDefaults ();
        }
        eEpromRead ();
        //  dumpConfig();
}

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

boolean DataStorage::isFirstRun ()
{
        //  return true;
        if (EEPROM.read( 100 ) != 255) return true;
        else return false;
}

void DataStorage::forceFirstRun ()
{
        EEPROM.write( 100, 0 );
        writeEEPROMDefaults ();
}

void DataStorage::writeEEPROMDefaults ()
{
        //defaults
        for ( unsigned int i = 0 ; i < EEPROM.length() ; i++)
        {
                EEPROM.write(i, 0);
        }

        for (byte i = 0; i < PWMS; i++ )
        {
                pwmSettings[i] = defaultPWMSettings[i];
                memcpy_P( &pwmSettings[i], &defaultPWMSettings[i], sizeof( pwmSettings[i]));
                writeEEPROMPWMConfig (i);
        }
        memcpy_P( &settings, &defaultSettings, sizeof( settings));
        writeEEPROMSettings ();
        EEPROM.write( 100, 255 );

}

void DataStorage::writeEEPROMForceNight ()
{
        EEPROM.write( 101, settings.forceNight);
}

void DataStorage::writeEEPROMForceOff ()
{
        EEPROM.write( 103, settings.forceOFF);
}

void DataStorage::writeEEPROMForceAmbient ()
{
        EEPROM.write( 102, settings.forceAmbient);
}

void DataStorage::writeEEPROMSettings ()
{
        EEPROM_writeAnything(101, settings);
}

void DataStorage::readEEPROMSettings ()
{
        EEPROM_readAnything (101, settings);
}

void DataStorage::eEpromRead( )
{
        for (byte i = 0; i < PWMS; i++)
        {
                EEPROM_readAnything (getEEPROMAddr( i )+2, pwmSettings[i]);
        }
        readEEPROMSettings ();
}

void DataStorage::writeEEPROMPWMConfig (byte pwmNumber)
{
        EEPROM_writeAnything (getEEPROMAddr( pwmNumber )+2, pwmSettings[pwmNumber]);

}

// kompatybilnosc z AQMA
int DataStorage::getEEPROMAddr( byte n )
{
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

void DataStorage::dumpConfig()
{
        int value;
        Serial.begin(115200);
        for ( unsigned int i = 0 ; i < 500 ; i++)
        {
                value = EEPROM.read(i);
                Serial.print(i);
                Serial.print("\t");
                Serial.print(value);
                Serial.println();
        }
        for (byte i = 0 ; i < PWMS ; i++)
        {
                Serial.println("PWM "+i);
                Serial.print("nightval ");
                Serial.println (pwmSettings[i].valueNight);
                Serial.print("isnight ");
                Serial.println (pwmSettings[i].isNightLight);

        }
}
