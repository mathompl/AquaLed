/**************************************************************
   AQUALED sensors and cooling fans header file (c) T. Formanowski 2016-2022
   https://github.com/mathompl/AquaLed
   GNU GENERAL PUBLIC LICENSE
**************************************************************/

#ifndef SENSORS_H
#define SENSORS_H
#include <Wire.h>
#include <OneWire.h>
#include <DS18B20.h>
#include "config.h"

#define ON true
#define OFF false

// sensors
#define LED_TEMPERATURE_FAN 0
#define SUMP_TEMPERATURE_FAN 1
#define WATER_TEMPERATURE_FAN 2

typedef struct {
        byte pin;
        float temperature;
        float nxTemperature;
        bool fanStatus;
        bool nxFanStatus;

} SENSORS_CONFIG;

typedef struct {
        byte address[8];
        boolean detected;
} SENSORS_LIST;


class Sensors
{
  public:
      Sensors ();
      void begin ();
      void requestReadings ();
      boolean readTemperatures ();
      void fansControl();
      void relaySwitch (byte sensor);
      byte listContains (byte addr[]);
      byte discoverOneWireDevices();
      SENSORS_LIST getList (byte sensor);
      SENSORS_CONFIG getConfig (byte fan);
      void invertFan (byte fan, boolean switchRelay);
      void setNXTemperature (byte fan, float newTemp) ;
      void setNXFanStatus (byte fan, boolean newStatus);
      double mapDouble(double x, double in_min, double in_max, double out_min, double out_max);

  private:
      OneWire *__oneWire;
      DS18B20 *__sensorsWire;
      SENSORS_LIST _list[7] = {0};
      SENSORS_CONFIG _config[3] = {0};
      void setupSensors ();
      void initFanPin (byte i, byte pin);
      void fansSwitch (byte sensor, byte max);
      void isDiscovered (byte &res, byte addr[]);
      void printAddr (byte addr[]);
};
#endif
