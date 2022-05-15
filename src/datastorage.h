#ifndef DATASTORAGE_H
#define DATASTORAGE_H
#include <EEPROM.h>
#include "aqualed.h"
#include "config.h"


class DataStorage
{
public:
      DataStorage ();
      void begin();
      void forceFirstRun ();
      void writeEEPROMDefaults ();
      void writeEEPROMForceNight ();
      void writeEEPROMForceOff ();
      void writeEEPROMForceAmbient ();
      void writeEEPROMSettings ();
      void readEEPROMSettings ();
      void eEpromRead();
      void writeEEPROMPWMConfig (byte pwmNumber);
      int getEEPROMAddr( byte n );
      void dumpConfig();
private:
      boolean isFirstRun ();
};
#endif
