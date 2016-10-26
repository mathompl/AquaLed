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

#include "aqualed.h"

void setup() {
        wdt_disable();
        writeEEPROMDefaults ();
        eEpromRead();
        setupPWMPins ();

#ifndef NO_TEMPERATURE
        setupSensors ();
#endif

#ifndef NO_BLUETOOTH
        bluetooth.begin(9600);
#endif

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
        readTime ();
        wdt_reset();

        // pwm
        pwm ();
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
#ifndef NO_TEMPERATURE
        fansControl ();
        wdt_reset();
#endif

        // read bluetooth
#ifndef NO_BLUETOOTH
        bluetoothServe ();
        wdt_reset();
#endif

}
