#include <Arduino.h>
#include "Aqualed.h"

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

     for config see:
     @aqualed.h
     @nextion.h
 */

void setup() {
        wdt_disable();
        writeEEPROMDefaults ();
        eEpromRead();
        setupPWMPins ();

#ifndef NO_TEMPERATURE
        setupSensors ();
#endif

#ifndef NO_BLUETOOTH
        setupBluetooth ();
#endif

#ifndef NO_NEXTION
        nexInit();
#endif

        // launch watchdog  - 4 seconds
      wdt_enable(WDTO_4S);

}

void loop() {
        // time
        readTime ();
        wdt_reset();

        // pwm
        pwm ();
        wdt_reset();

        // nextion routines
#ifndef NO_NEXTION
        // nextion display
        nxDisplay ();
        wdt_reset();
        // nextion touch istener
        nxTouch();
        wdt_reset();
#endif


        // temperature and fans control
#ifndef NO_TEMPERATURE
        fansControl ();
        wdt_reset();
#endif

        // bluetooth routines
#ifndef NO_BLUETOOTH
        bluetoothServe ();
        wdt_reset();
#endif

}
