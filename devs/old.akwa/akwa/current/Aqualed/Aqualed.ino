
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
#include <OneWire.h>
#include <avr/wdt.h>
#include <DS18B20.h>
#include "aqualed.h"
#include <SoftwareSerial.h>
#include "nextion.h"
#include <Adafruit_PWMServoDriver.h>


SoftwareSerial bluetooth(A2, A1);
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);
Adafruit_PWMServoDriver pwm_i2c = Adafruit_PWMServoDriver();

void setup() {
  wdt_disable();

#ifdef BLUETOOTH
  bluetooth.begin(9600);
#endif

  writeEEPROMDefaults ();
  eEpromRead();
  setupPWMPins ();
  //    delay(1000L);
  setupSensors ();
#ifdef NEXTION
  nexInit();
#endif

#ifdef I2C
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

    /*
      // DST
      if (pl_TZ.locIsDST(now( )) && !SETTINGS.dst)
      {
         setTime( tm.Hour-1, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year); // godz, min, sek, dzien, miesiac, rok
         RTC.set( now( ) );
      }
      if (!pl_TZ.locIsDST(now( )) && SETTINGS.dst)
      {
         setTime( tm.Hour+1, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year); // godz, min, sek, dzien, miesiac, rok
         RTC.set( now( ) );
      }    */

    currentTimeSec = (long(tm.Hour) * 3600) + (long(tm.Minute) * 60) + long(tm.Second);
  }
  wdt_reset();


  // obsluga nextiona
#ifdef NEXTION
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
#ifdef BLUETOOTH
  bluetoothServe ();
  wdt_reset();
#endif

  // pwm
  pwm ();
  wdt_reset();

}

