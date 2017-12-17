/*
   AQUALED Temperature && fans relay  functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#ifndef NO_TEMPERATURE

#include <Arduino.h>
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensorsWire(&onewire);

static void setupSensors ()
{
        sensorsWire.begin();
        initFanPin (LED_TEMPERATURE_FAN, LED_FANS_PIN);
        initFanPin (WATER_TEMPERATURE_FAN, WATER_FANS_PIN);
        initFanPin (SUMP_TEMPERATURE_FAN, SUMP_FANS_PIN);
        requestReadings ();
}

static void initFanPin (byte i, byte pin)
{
        sensors[i].pin = pin;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
}

static void requestReadings ()
{
        for (byte i = 0; i < 3; i++)
                sensorsWire.request(settings.sensors[i]);
}

static void fansControl()
{
        if (currentMillis - previousTemperature < TEMPERATURE_SAMPLE_INTERVAL) return;
        previousTemperature = currentMillis;

        if (sensorsWire.available()) {
                for (byte i = 0; i < 3; i++)
                        sensors[i].temperature = sensorsWire.readTemperature(settings.sensors[i]);

                requestReadings ();
        }

        // lamp overheating (for eg due to fans failure)
        if (sensors[LED_TEMPERATURE_FAN].temperature >= LAMP_TEMPERATURE_MAX ||
            sensors[SUMP_TEMPERATURE_FAN].temperature >= LAMP_TEMPERATURE_MAX) lampOverheating = true;
        else lampOverheating = false;

        if (currentMillis - previousMillisFans > FANS_INTERVAL || previousMillisFans == 0)
        {
                previousMillisFans = currentMillis;
                for (byte i = 0; i < 3; i++)
                        fansSwitch (i, sensors[i].pin,settings.maxTemperatures[i]);
        }
}

static void fansSwitch (byte sensor, byte pin, byte max)
{
        if (sensors[sensor].temperature != TEMP_ERROR && sensors[sensor].temperature > (double) max )
        {
                digitalWrite(pin, LOW);
                sensors[sensor].fanStatus = true;
        }
        else
        {
                digitalWrite(pin, HIGH);
                sensors[sensor].fanStatus = false;
        }
}

static byte listContains (byte addr[])
{
        for (byte i = 0; i < 7; i++)
                if(memcmp (sensorsList[i].address, addr, 8) == 0) return i;

        return 255;
}

static void isDiscovered (byte &res, byte addr[])
{
        //if (checkAddr(addr) && listContains (addr) == 255)
        if (listContains (addr) == 255)
        {
                memcpy( sensorsList[res].address, addr, 8);
                sensorsList[res].detected = false;
                res++;
        }
}

static byte discoverOneWireDevices() {
        byte res = 0;
        byte addr[8];
        // pierwszy pusty

        memset (sensorsList, 0, sizeof (sensorsList));

        // pierwszy pusty
        res++;

        while (onewire.search(addr)) {
                if ( OneWire::crc8( addr, 7) != addr[7] ) continue;
                memcpy( sensorsList[res].address, addr, 8);
                sensorsList[res].detected = true;
                res++;
        }
        for (byte i = 0; i < 3; i++)
                isDiscovered (res, settings.sensors[i]);

        onewire.reset_search();
        return res;
}
#endif
