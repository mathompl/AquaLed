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
        sensors.request(settings.ledSensorAddress);
        sensors.request(settings.waterSensorAddress);
        sensors.request(settings.sumpSensorAddress);
}

void fansControl()
{

        if (currentMillis - previousTemperature < TEMPERATURE_SAMPLE_INTERVAL) return;
        previousTemperature = currentMillis;

        if (sensors.available()) {
                temperaturesFans[LED_TEMPERATURE_FAN].temperature = sensors.readTemperature(settings.ledSensorAddress);
                temperaturesFans[WATER_TEMPERATURE_FAN].temperature = sensors.readTemperature(settings.waterSensorAddress);
                temperaturesFans[SUMP_TEMPERATURE_FAN].temperature = sensors.readTemperature(settings.sumpSensorAddress);;
                requestReadings ();
        }

        // lamp overheating (for eg due to fans failure)
        if (temperaturesFans[LED_TEMPERATURE_FAN].temperature >= LAMP_TEMPERATURE_MAX ||
            temperaturesFans[SUMP_TEMPERATURE_FAN].temperature >= LAMP_TEMPERATURE_MAX) lampOverheating = true;
        else lampOverheating = false;

        if (currentMillis - previousMillisFans > FANS_INTERVAL || previousMillisFans == 0)
        {
                previousMillisFans = currentMillis;

                fansSwitch (LED_TEMPERATURE_FAN, LED_FANS_PIN,settings.max_led_temp);
                fansSwitch (SUMP_TEMPERATURE_FAN, SUMP_FANS_PIN,settings.max_sump_temp);
                fansSwitch (WATER_TEMPERATURE_FAN, WATER_FANS_PIN,settings.max_water_temp);
        }

}

void fansSwitch (byte sensor, byte pin, byte max)
{
        if (temperaturesFans[sensor].temperature != TEMP_ERROR && temperaturesFans[sensor].temperature > max )
        {
                digitalWrite(pin, LOW);
                temperaturesFans[sensor].fanStatus = true;
        }
        else
        {
                digitalWrite(pin, HIGH);
                temperaturesFans[sensor].fanStatus = false;
        }
}

byte listContains (byte addr[])
{
        for (byte i = 0; i < 7; i++)
          if(memcmp (sensorsList[i].address, addr, 8) == 0) return i;

        return 255;
}

void isDiscovered (byte &res, byte addr[])
{
        //if (checkAddr(addr) && listContains (addr) == 255)
        if (listContains (addr) == 255)
        {
                for (byte k = 0; k < 8; k++)
                {
                          sensorsList[res].address[k] = addr[k];
                          sensorsList[res].detected = false;
                }
                res++;
        }
}

byte discoverOneWireDevices() {
        byte res = 0;
        byte addr[8];
        // pierwszy pusty

        memset (sensorsList, 0, sizeof (sensorsList));

        // pierwszy pusty
        for (byte k = 0; k < 8; k++)
        {
                sensorsList[res].address[k] = 0;
                sensorsList[res].detected = false;
        }
        res++;

        while (onewire.search(addr)) {
                if ( OneWire::crc8( addr, 7) != addr[7] ) continue;

                for (byte k = 0; k < 8; k++)
                {
                        sensorsList[res].address[k] = addr[k];
                        sensorsList[res].detected = true;
                }
                res++;
        }
        isDiscovered (res, settings.ledSensorAddress);
        isDiscovered (res, settings.sumpSensorAddress);
        isDiscovered (res, settings.waterSensorAddress);
        onewire.reset_search();
        return res;
}
#endif
