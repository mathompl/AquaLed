#include "aqualed.h"


/*
    AQUALED Arduino PWM LED lamp driver software with LCD support and temperature control for marine, reef or sweetwater aquariums.

    Features:

    - 8 channel PWM control of LED modules (drivers), with I2C module up to 512 (up to 8 channels on LCD)
    - programmable light hours with sunsets and sunrises
    - night light support
    - 3 digital thermometers with asynchronous temperature read, for: water, lamp and addictional lamp or sump, controlling 3 separate relays (cooling fans)
    - customizable dimming, linear or logarithmic algorith
    - full configuration and status using Nextion 2.8" LCD with touch support, including: operational hours, sunset & sunrise hours, thermometers, dimming times, schedule, night mode etc.
    - override programs: off, night and user accessible from LCD
    - date/time support with RTC module
    - screen saver with water temperature and time
    - test mode
    - all settings stored in EEPROM

     (c) 2016 - 2022 Tomek Formanowski mathom@pifpaf.pl
     GNU GENERAL PUBLIC LICENSE

     Use at your own risk.

     for config see:

     @defaults.h
     @config.h
     @nextion.h

     published on github:
     https://github.com/mathompl/AquaLed
 */


void setup() {
  #ifdef ENABLE_WATCHDOG
        wdt_disable();
  #endif

// read EEPROM and write defaults if first run
       dataStorage.begin();

// configure RTC
        time.begin ();

// configure and connect NEXTION lcd
        nextion.begin();

// configure PWM output
        pwm.begin ();

// configure temperature sensors and fans pins
        sensors.begin ();

#ifdef ENABLE_WATCHDOG
        wdt_enable(WDTO_2S);
#endif

}


void loop() {
        // time
        time.read ();

        // pwm
        pwm.loop ();

        // in case of nextion disconnect / baud rate change
        nextion.keepAlive();
        // nextion touch istener
        nextion.listen();
        // nextion display
        nextion.display();

        // temperature and fans control
        sensors.readTemperatures();
        sensors.fansControl ();

#ifdef ENABLE_WATCHDOG
        wdt_reset();
#endif

}
