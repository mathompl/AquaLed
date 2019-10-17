
#include <Wire.h>
#include <Time.h>
#include <EEPROM.h>
#include <DS1307RTC.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <avr/wdt.h>
#include "aqma.h"
#include "DS18B20.h"

#include "NexTion.h"
//#include "NexText.h"
//#include "NexPage.h"
//#include "NexPicture.h"

SoftwareSerial bluetooth(A2, A1);
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);

void setup() {
    wdt_disable();

    delay(2L * 1000L);

    sensors.begin();

    pinMode(FANS_PIN, OUTPUT); 
    pinMode(AQUA_PIN, OUTPUT);
    digitalWrite(FANS_PIN, HIGH);
    digitalWrite(AQUA_PIN, HIGH);

    // The first requests sensor for measurement
    sensors.request(sensorAddressFans);
    sensors.request(sensorAddressAqua);


    Serial.begin(9600);
    bluetooth.begin(9600);


    pinMode(pwm1Pin, OUTPUT);
    pinMode(pwm2Pin, OUTPUT);
    pinMode(pwm3Pin, OUTPUT);
    pinMode(pwm4Pin, OUTPUT);
    pinMode(pwm5Pin, OUTPUT);
    pinMode(pwm6Pin, OUTPUT);


    // Ustaw stany poczatkowe 
    digitalWrite(pwm1Pin, OFF);
    digitalWrite(pwm2Pin, OFF);
    digitalWrite(pwm3Pin, OFF);
    digitalWrite(pwm4Pin, OFF);
    digitalWrite(pwm5Pin, OFF);
    digitalWrite(pwm6Pin, OFF);

    //TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); 

    TCCR1A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
    TCCR2A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
    //TCCR2B = _BV(CS00);

    eEpromRead();

    setupPopups ();
    
  
    wdt_enable(WDTO_4S);

}

void loop() {
    long unsigned currentMillis = millis();
   if (RTC.read(tm)) {
        currentTimeSec = (long(tm.Hour)*3600) + (long(tm.Minute)*60) + long(tm.Second);
    }

    wdt_reset();
// nextion      
    if (nxScreen ==0 )
    {
      timeDisplay(tm);
      updateInfo();
    }
    nexTouch ();

    wdt_reset();
    char cmdChar = 0;
    String cmd = "";

 


    // If the sesor measurement is ready, print the results
    if (sensors.available()) {
        // Reads the temperature from sensor
        temperatureFans = sensors.readTemperature(sensorAddressFans);
        temperatureAqua = sensors.readTemperature(sensorAddressAqua);

        // Prints the temperature on Serial Monitor
        if (temperatureFans != TEMP_ERROR) {
            if (currentTimeSec - previousMillisFans > FANS_INTERVAL) {
                previousMillisFans = currentTimeSec;
                if (temperatureFans > MAX_BULB_TEMP)
                {
                    digitalWrite(FANS_PIN, LOW);
                    ledFansStatus = true;
  
                }
                else
                {
                    digitalWrite(FANS_PIN, HIGH);
                    waterFansStatus = false;
                }
            }

        }

        if (temperatureAqua != TEMP_ERROR) {
            if (currentTimeSec - previousMillisAqua > AQUA_INTERVAL) {
                previousMillisAqua = currentTimeSec;
                if (temperatureAqua > MAX_AQUA_TEMP)
                {
                    digitalWrite(AQUA_PIN, LOW);
                    waterFansStatus = true;
                }
                else
                {
                    digitalWrite(AQUA_PIN, HIGH);
                    waterFansStatus = false;
                }
            }
        }

        // Another requests sensor for measurement
        sensors.request(sensorAddressFans);
        sensors.request(sensorAddressAqua);
    }


    // Serial
/*
    while (Serial.available() > 0) {
        cmdChar = Serial.read();
        delay(5);
        cmd.concat(cmdChar);
    }*/


    while (bluetooth.available() > 0) {
        cmdChar = bluetooth.read();
        delay(5);
        cmd.concat(cmdChar);
    }

    if (cmd != "") {

        cmd.toCharArray(cmdOutputArray, 64);

        if (commandAnalysis(cmdOutputArray)) {
            eEpromRead();
        }

        else {
            //             Serial.print("666,Bledne dane\n"); 
            bluetooth.print("666,Bledne dane\n");
        }

    }

    cmd = "";


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////     
    //////////////////////////////////////// SILNIK //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   silkySmoothPowerOn(pwmSilkySmootTimeSec, currentMillis);

    // PWM 1 - 6
    if (currentMillis % 4 == 0) {
        led1status = pwm(pwm1Pin, pwm1Status, pwm1HOn, pwm1MOn, pwm1SOn, pwm1HOff, pwm1MOff, pwm1SOff, pwm1Min, pwm1Max, pwm1Sr, pwm1Ss, pwm1KeepLight, pwm1Invert);
    }
    if (currentMillis % 5 == 0) {
        led2status = pwm(pwm2Pin, pwm2Status, pwm2HOn, pwm2MOn, pwm2SOn, pwm2HOff, pwm2MOff, pwm2SOff, pwm2Min, pwm2Max, pwm2Sr, pwm2Ss, pwm2KeepLight, pwm2Invert);
    }
    if (currentMillis % 6 == 0) {
        led3status = pwm(pwm3Pin, pwm3Status, pwm3HOn, pwm3MOn, pwm3SOn, pwm3HOff, pwm3MOff, pwm3SOff, pwm3Min, pwm3Max, pwm3Sr, pwm3Ss, pwm3KeepLight, pwm3Invert);
    }
    if (currentMillis % 7 == 0) {
        led4status = pwm(pwm4Pin, pwm4Status, pwm4HOn, pwm4MOn, pwm4SOn, pwm4HOff, pwm4MOff, pwm4SOff, pwm4Min, pwm4Max, pwm4Sr, pwm4Ss, pwm4KeepLight, pwm4Invert);
    }
    if (currentMillis % 8 == 0) {
        led5status = pwm(pwm5Pin, pwm5Status, pwm5HOn, pwm5MOn, pwm5SOn, pwm5HOff, pwm5MOff, pwm5SOff, pwm5Min, pwm5Max, pwm5Sr, pwm5Ss, pwm5KeepLight, pwm5Invert);
    }
    if (currentMillis % 9 == 0) {
        led6status = pwm(pwm6Pin, pwm6Status, pwm6HOn, pwm6MOn, pwm6SOn, pwm6HOff, pwm6MOff, pwm6SOff, pwm6Min, pwm6Max, pwm6Sr, pwm6Ss, pwm6KeepLight, pwm6Invert);
      
    }


    

  
 
}

////////////////////////////////////////////////////////////// END LOOP////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////















