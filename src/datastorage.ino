#include "datastorage.h"
#include "defaults.h"

#define EEPROM_CONFIG_EXISTS_ADDR 1
#define EEPROM_SETTINGS_ADDR 2
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

boolean DataStorage::isFirstRun ()
{
        //  return true;
        if (EEPROM.read( EEPROM_CONFIG_EXISTS_ADDR ) != 255) return true;
        else return false;
}

void DataStorage::forceFirstRun ()
{
        EEPROM.write( EEPROM_CONFIG_EXISTS_ADDR, 0 );
}

void DataStorage::writeEEPROMDefaults ()
{
        //defaults
        for ( unsigned int i = EEPROM_CONFIG_EXISTS_ADDR+1 ; i < EEPROM.length() ; i++)
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
        EEPROM.write( EEPROM_CONFIG_EXISTS_ADDR, 255 );

}

void DataStorage::writeEEPROMForceNight ()
{
        EEPROM.write( EEPROM_SETTINGS_ADDR, settings.forceNight);
}

void DataStorage::writeEEPROMForceOff ()
{
        EEPROM.write( EEPROM_SETTINGS_ADDR+2, settings.forceOFF);
}

void DataStorage::writeEEPROMForceAmbient ()
{
        EEPROM.write( EEPROM_SETTINGS_ADDR+1, settings.forceAmbient);
}

void DataStorage::writeEEPROMSettings ()
{
        EEPROM.put (EEPROM_SETTINGS_ADDR, settings);
}

void DataStorage::readEEPROMSettings ()
{
        EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
}

void DataStorage::eEpromRead( )
{
        for (byte i = 0; i < PWMS; i++)
        {
                EEPROM.get (getEEPROMAddr( i ), pwmSettings[i]);
        }
        readEEPROMSettings ();
}

void DataStorage::writeEEPROMPWMConfig (byte pwmNumber)
{
        EEPROM.put (getEEPROMAddr( pwmNumber ), pwmSettings[pwmNumber]);
}

int DataStorage::getEEPROMAddr( byte n )
{
        return EEPROM_SETTINGS_ADDR + sizeof (SETTINGS) + (n * sizeof (PWM_SETTINGS) + 1);
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
