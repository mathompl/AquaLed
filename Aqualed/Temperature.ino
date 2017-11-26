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
                temperatureLed = sensors.readTemperature(SETTINGS.ledSensorAddress);
                temperatureWater = sensors.readTemperature(SETTINGS.waterSensorAddress);
                temperatureSump = sensors.readTemperature(SETTINGS.sumpSensorAddress);;
                requestReadings ();
        }

        // lamp overheating (for eg due to fans failure)
        if (temperatureLed >= LAMP_TEMPERATURE_MAX || temperatureSump >= LAMP_TEMPERATURE_MAX) lampOverheating = true;
        else lampOverheating = false;

        if (currentMillis - previousMillisFans > FANS_INTERVAL || previousMillisFans == 0) {
                previousMillisFans = currentMillis;
                if (temperatureLed != TEMP_ERROR &&temperatureLed > SETTINGS.max_led_temp)
                {
                        digitalWrite(LED_FANS_PIN, LOW);
                        ledFansStatus = true;
                }
                else
                {
                        digitalWrite(LED_FANS_PIN, HIGH);
                        ledFansStatus = false;
                }

                if (temperatureSump != TEMP_ERROR && temperatureSump > SETTINGS.max_sump_temp)
                {
                        digitalWrite(SUMP_FANS_PIN, LOW);
                        sumpFansStatus = true;
                }
                else
                {
                        digitalWrite (SUMP_FANS_PIN, HIGH);
                        sumpFansStatus = false;
                }

                if (temperatureWater != TEMP_ERROR && temperatureWater > SETTINGS.max_water_temp )
                {
                        digitalWrite(WATER_FANS_PIN, LOW);
                        waterFansStatus = true;
                }
                else
                {
                        digitalWrite(WATER_FANS_PIN, HIGH);
                        waterFansStatus = false;
                }
        }


}

static boolean checkAddr (byte s[])
{
        if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[3] == 0 && s[4] == 0 && s[5] == 0 && s[6] == 0 && s[7] == 0) return false; else return true;
}


/*
   byte printSensorAddr (byte s[])
   {

   for (int k = 0 ; k < 8 ; k++)
   {
    Serial.print(s[k], HEX);
    if (k < 7) Serial.print(", ");

   }
   Serial.println("");
   }*/

byte listContains (byte addr[])
{
        for (byte i = 0; i < 7; i++)
                if (sensorsList[i][0] == addr[0] && sensorsList[i][1] == addr[1] && sensorsList[i][2] == addr[2] &&
                    sensorsList[i][3] == addr[3] && sensorsList[i][4] == addr[4] && sensorsList[i][5] == addr[5] &&
                    sensorsList[i][6] == addr[6] && sensorsList[i][7] == addr[7] )
                        return i;
        return 255;
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

        if (checkAddr(SETTINGS.ledSensorAddress) && listContains (SETTINGS.ledSensorAddress) == 255)
        {

                for (byte k = 0; k < 8; k++)
                {
                        sensorsList[res][k] = SETTINGS.ledSensorAddress[k];
                        sensorsDetected[res] = false;
                }

                res++;
        }
        if (checkAddr(SETTINGS.sumpSensorAddress) && listContains (SETTINGS.sumpSensorAddress) == 255)
        {
                for (byte k = 0; k < 8; k++)
                {
                        sensorsList[res][k] = SETTINGS.sumpSensorAddress[k];
                        sensorsDetected[res] = false;
                }
                res++;
        }
        if (checkAddr(SETTINGS.waterSensorAddress) && listContains (SETTINGS.waterSensorAddress) == 255)
        {
                for (byte k = 0; k < 8; k++)
                {
                        sensorsList[res][k] = SETTINGS.waterSensorAddress[k];
                        sensorsDetected[res] = false;
                }
                res++;
        }
        onewire.reset_search();
        return res;
}
#endif
