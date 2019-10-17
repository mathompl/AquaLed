#include <Wire.h>
#include <Time.h>
#include <EEPROM.h>
#include <DS1307RTC.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <avr/wdt.h>
#include "DS18B20.h"
#include "aqualed.h"
#include "nextion.h"
#include <stdarg.h>





SoftwareSerial bluetooth(A2, A1);
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);

void setup() {
    wdt_disable();

    delay(2L * 1000L);
  

    setupSensors ();
    Serial.begin(9600);
    bluetooth.begin(9600);


    setupLedPins ();
    eEpromRead();

   

    nexInit();
    wdt_enable(WDTO_4S);

}

void loop() {

// time
   currentMillis = millis();
   if (RTC.read(tm)) {
        currentTimeSec = (long(tm.Hour)*3600) + (long(tm.Minute)*60) + long(tm.Second);
    }
    wdt_reset();
    
// nextion      
    nxCore ();
    nxTouch();
    wdt_reset();
    
// fans && temp
    fansControl ();
    wdt_reset();

// read bluetooth
    bluetoothCore ();
    wdt_reset();

// ledcore
    ledCore (); 
}








