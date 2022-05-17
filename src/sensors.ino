/**************************************************************
   AQUALED tempearature sensors and cooling fans relays library (c) T. Formanowski 2016-2022
   https://github.com/mathompl/AquaLed
   GNU GENERAL PUBLIC LICENSE
**************************************************************/

#include <sensors.h>

Sensors::Sensors () {
}

void Sensors::begin()
{
        __oneWire = new OneWire (ONEWIRE_PIN);
        __sensorsWire = new DS18B20 (__oneWire);
        __sensorsWire->begin();
        setupSensors ();
        requestReadings ();
}

void Sensors::setupSensors ()
{
        initFanPin (LED_TEMPERATURE_FAN, LED_FANS_PIN);
        initFanPin (WATER_TEMPERATURE_FAN, WATER_FANS_PIN);
        initFanPin (SUMP_TEMPERATURE_FAN, SUMP_FANS_PIN);
}

void Sensors::initFanPin (byte i, byte pin)
{
        _config[i].pin = pin;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
}

void Sensors::printAddr (byte addr[])
{
        for (byte b = 0; b < 8; b++)
        {
                Serial.print (addr[b], HEX);
                Serial.print (" ");
        }
}

boolean Sensors::readTemperatures ()
{
        if (currentMillis - previousTemperatureRead < TEMPERATURE_READ_INTERVAL ) return false;
        previousTemperatureRead = currentMillis;

        if (__sensorsWire->available())
        {
                for (byte i = 0; i < 3; i++)
                {
                        if (settings.sensors[i][0]==0) continue;
                        _config[i].temperature = __sensorsWire->readTemperature(settings.sensors[i]);
                }
        }
        // for some reason, this has to be called right after reading temperatures, if run in a separate intervals, DS18B20->avaolable () always returns false
        requestReadings ();
        return true;
}

// asynchronous RTC pool
void Sensors::requestReadings ()
{
        for (byte i = 0; i < 3; i++)
        {
                if (settings.sensors[i][0]==0) continue;
                __sensorsWire->request(settings.sensors[i]);
        }
}

void Sensors::fansControl()
{
        if (currentMillis - previousMillisFans > FANS_INTERVAL || previousMillisFans == 0)
        {
                previousMillisFans = currentMillis;
                for (byte i = 0; i < 3; i++)
                {
                        fansSwitch (i, settings.maxTemperatures[i]);
                        relaySwitch (i);
                }
        }
}

void Sensors::relaySwitch (byte sensor)
{
        if (_config[sensor].fanStatus)
                digitalWrite(_config[sensor].pin, LOW);
        else
                digitalWrite(_config[sensor].pin, HIGH);
}

void Sensors::fansSwitch (byte sensor, byte max)
{
        if (_config[sensor].temperature == TEMP_ERROR || _config[sensor].temperature > (double) max )
                _config[sensor].fanStatus = true;
        else
                _config[sensor].fanStatus = false;
}

byte Sensors::listContains (byte addr[])
{
        for (byte i = 0; i < 7; i++)
                if(memcmp (_list[i].address, addr, 8) == 0) return i;

        return 255;
}

void Sensors::isDiscovered (byte &res, byte addr[])
{
        //if (checkAddr(addr) && listContains (addr) == 255)
        if (listContains (addr) == 255)
        {
                memcpy( _list[res].address, addr, 8);
                _list[res].detected = false;
                res++;
        }
}

byte Sensors::discoverOneWireDevices() {
        byte res = 0;
        byte addr[8];

        __oneWire->reset ();
        __oneWire->reset_search();
        memset (_list, 0, sizeof (_list));
        // first {0,0,0,0,0,0,0,0}
        res++;
        while (__oneWire->search(addr)) {
                if ( __oneWire->crc8( addr, 7) != addr[7] ) continue;
                memcpy( _list[res].address, addr, 8);
                _list[res].detected = true;
                res++;
        }
        for (byte i = 0; i < 3; i++)
                isDiscovered (res, settings.sensors[i]);

        __oneWire->reset_search();
        return res;
}

SENSORS_LIST Sensors::getList (byte sensor)
{
        return _list[sensor];
}
SENSORS_CONFIG Sensors::getConfig (byte fan)
{
        return _config [fan];
}
void Sensors::invertFan (byte fan, boolean switchRelay)
{
        _config[fan].fanStatus = !_config[fan].fanStatus;
        if (switchRelay) relaySwitch (fan);
}

void Sensors::setNXTemperature  (byte fan, float newTemp)
{
        _config[fan].nxTemperature = newTemp;
}

void Sensors::setNXFanStatus (byte fan, boolean newStatus)
{
        _config[fan].nxFanStatus = newStatus;
}
