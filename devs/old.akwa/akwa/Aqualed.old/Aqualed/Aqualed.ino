#include <Arduino.h>

/*
      AquaLed - sterownik oswietlenia akwarium morskiego v1.0
       - max 8 PWM obslugiwanych przez wyswietlacz, teoretycznie do 992 kanalow PWM na sterowniku PCA9685
       - 3 czujnki termeratury,
       - 3 przekazniki na wentylatory
       - wyswietlacz Nextion 2.8" (wersja standard)

     (c) 2016 Tomek Formanowski mathom@pifpaf.pl
     Open Source public domain

     Fragmenty kodu: bluetooth ze sterownika Aqma by Magu, kombatybilnosc zachowana w zakresie obslugi przez bluetooth

     Autor nie bierze odpowiedzialnosci za utrate danych lub inne szkody wyrzadzone przez kod programu lub bledne uzytkowanie


 */
#include <Wire.h>
#include <Time.h>
#include <EEPROM.h>
#include <DS1307RTC.h>
#include <avr/pgmspace.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <Adafruit_PWMServoDriver.h>
#include <OneWire.h>
#include <avr/wdt.h>
#include <DS18B20.h>

#include "aqualed.h"
#include "nextion.h"


SoftwareSerial bluetooth(A2, A1);
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);
Adafruit_PWMServoDriver pwm_i2c;

void setup() {
        wdt_disable();

#ifndef NO_BLUETOOTH
        bluetooth.begin(9600);
#endif

        writeEEPROMDefaults ();
        eEpromRead();
        setupPWMPins ();
        //    delay(1000L);
        setupSensors ();
#ifndef NO_NEXTION
        nexInit();
#endif

#ifndef NO_I2C
        pwm_i2c = Adafruit_PWMServoDriver();
        pwm_i2c.begin();
        pwm_i2c.setPWMFreq(1500);
#endif

        // watchdog na 4 sekundy
        wdt_enable(WDTO_4S);

}

void loop() {


        // time
        currentMillis = millis();
        if (RTC.read(tm)) {
                currentTimeSec = (long(tm.Hour) * 3600) + (long(tm.Minute) * 60) + long(tm.Second);
                adjustDST ();
        }
        wdt_reset();


        // obsluga nextiona
#ifndef NO_NEXTION
        // obsluga wyswietlania
        nxDisplay ();
        wdt_reset();
        // obsluga dotyku
        nxTouch();
        wdt_reset();
#endif


        // kontrola temperatury i wiatrakow
        fansControl ();
        wdt_reset();

        // read bluetooth
#ifndef NO_BLUETOOTH
        bluetoothServe ();
        wdt_reset();
#endif

        // pwm
        pwm ();
        wdt_reset();
}
