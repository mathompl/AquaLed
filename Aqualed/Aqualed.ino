#include <Arduino.h>
#include "Aqualed.h"

/*
    AQUALED Arduino PWM LED lamp driver software with LCD support and temperature control for marine, reef or sweetwater aquariums.

    Features:

    - 6 channel PWM control of LED modules (drivers), with I2C module up to 512 (up to 8 channels on LCD)
    - programmable light hours with sunsets and sunrises
    - night light support
    - 3 digital thermometers with asynchronous temperature read, for: water, lamp and addictional lamp or sump, controlling 3 separate relays (cooling fans)
    - customizable dimming, linear or logarithmic algorith
    - bluetooth compatibility with AQma Led Control by Maqu http://magu.pl/aqma-led-control
    - full configuration and status using Nextion 2.8" LCD with touch support, including: operational hours, sunset & sunrise hours, thermometers, dimming times, schedule, night mode etc.
    - override programs: off, night and user accessible from LCD
    - date/time support with RTC module
    - screen saver with water temperature and time
    - test mode
    - all settings stored in EEPROM

     (c) 2016 - 2017 Tomek Formanowski mathom@pifpaf.pl
     Open Source public domain

     Use at your own risk.

     for config see:
     @aqualed.h
     @nextion.h

     published on github:
     https://github.com/mathompl/AquaLed
 */

void setup() {
        wdt_disable();
        writeEEPROMDefaults ();
        eEpromRead();
        rtcSetup ();
        
        getMoonPhase ();

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
        wdt_enable(WDTO_8S);
        setupPWMPins ();
}

void loop() {
        // time
        readTime ();

        // pwm
        pwm ();

        // nextion routines
#ifndef NO_NEXTION
        // nextion display
        nxDisplay ();
        // nextion touch istener
        nxTouch();
#endif

        // temperature and fans control
#ifndef NO_TEMPERATURE
        fansControl ();
#endif

        // bluetooth routines
#ifndef NO_BLUETOOTH
        bluetoothServe ();
#endif
        wdt_reset();
}
