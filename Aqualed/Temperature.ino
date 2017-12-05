/*
   AQUALED Temperature && fans relay  functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#ifndef NO_TEMPERATURE

#include <Arduino.h>
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);

void setupSensors ()
{
        sensors.begin();
        pinMode(LED_FANS_PIN, OUTPUT);
        pinMode(WATER_FANS_PIN, OUTPUT);
        pinMode(SUMP_FANS_PIN, OUTPUT);
        digitalWrite(LED_FANS_PIN, HIGH);
        digitalWrite(WATER_FANS_PIN, HIGH);
        digitalWrite(SUMP_FANS_PIN, HIGH);
        requestReadings ();
}

static void requestReadings ()
{
        sensors.request(SETTINGS.ledSensorAddress);
        sensors.request(SETTINGS.waterSensorAddress);
        sensors.request(SETTINGS.sumpSensorAddress);
}

void fansControl()
{

        if (currentMillis - previousTemperature < TEMPERATURE_SAMPLE_INTERVAL) return;
        previousTemperature = currentMillis;

        if (sensors.available()) {
                temperatures[LED_TEMPERATURE_FAN] = sensors.readTemperature(SETTINGS.ledSensorAddress);
                temperatures[WATER_TEMPERATURE_FAN] = sensors.readTemperature(SETTINGS.waterSensorAddress);
                temperatures[SUMP_TEMPERATURE_FAN] = sensors.readTemperature(SETTINGS.sumpSensorAddress);;
                requestReadings ();
        }

        // lamp overheating (for eg due to fans failure)
        if (temperatures[LED_TEMPERATURE_FAN] >= LAMP_TEMPERATURE_MAX || temperatures[SUMP_TEMPERATURE_FAN] >= LAMP_TEMPERATURE_MAX) lampOverheating = true;
        else lampOverheating = false;

        if (currentMillis - previousMillisFans > FANS_INTERVAL || previousMillisFans == 0)
        {
                previousMillisFans = currentMillis;

                fansSwitch (LED_TEMPERATURE_FAN, LED_FANS_PIN,SETTINGS.max_led_temp);
                fansSwitch (SUMP_TEMPERATURE_FAN, SUMP_FANS_PIN,SETTINGS.max_sump_temp);
                fansSwitch (WATER_TEMPERATURE_FAN, WATER_FANS_PIN,SETTINGS.max_water_temp);
        }

}

void fansSwitch (byte sensor, byte pin, byte max)
{
        if (temperatures[sensor] != TEMP_ERROR && temperatures[sensor] > max )
        {
                digitalWrite(pin, LOW);
                fansStatuses[sensor] = true;
        }
        else
        {
                digitalWrite(pin, HIGH);
                fansStatuses[sensor] = false;
        }
}

byte listContains (byte addr[])
{
        for (byte i = 0; i < 7; i++)
          if(memcmp (sensorsList[i], addr, 8) == 0) return i;

        return 255;
}

void isDiscovered (byte &res, byte addr[])
{
        //if (checkAddr(addr) && listContains (addr) == 255)
        if (listContains (addr) == 255)
        {
                for (byte k = 0; k < 8; k++)
                {
                        sensorsList[res][k] = addr[k];
                        sensorsDetected[res] = false;
                }
                res++;
        }
}

byte discoverOneWireDevices() {
        byte res = 0;
        byte addr[8];
        // pierwszy pusty

        memset (sensorsList, 0, 56);

        // pierwszy pusty
        for (byte k = 0; k < 8; k++)
        {
                sensorsList[res][k] = 0;
                sensorsDetected[res] = false;
        }
        res++;

        while (onewire.search(addr)) {
                if ( OneWire::crc8( addr, 7) != addr[7] ) continue;

                for (byte k = 0; k < 8; k++)
                {
                        sensorsList[res][k] = addr[k];
                        sensorsDetected[res] = true;
                }
                res++;
        }
        isDiscovered (res, SETTINGS.ledSensorAddress);
        isDiscovered (res, SETTINGS.sumpSensorAddress);
        isDiscovered (res, SETTINGS.waterSensorAddress);
        onewire.reset_search();
        return res;
}
#endif
