/*
 AQUALED user configuration file (c) T. Formanowski 2016-2017
https://github.com/mathompl/AquaLed
*/

#include <Arduino.h>
#include <avr/pgmspace.h>

/*
    USER DEFINED SETTINGS
*/
// modules, uncomment to disable, comment to enable
// uncomment to use I2C PWM MODULE, PINS CONFIGURATION FOLLOWS

#define NO_BLUETOOTH
//#define NO_NEXTION
//#define NEXTION_SOFTWARE_SERIAL
//#define NO_TEMPERATURE
//#define DEBUG

//  PWM channels
#define PWMS 8

// PWM channels displayed names
#define PWM_1_NAME "Cool white 1\0"
#define PWM_2_NAME "Cool white 2\0"
#define PWM_3_NAME "Royal Blue 1\0"
#define PWM_4_NAME "Royal Blue 2\0"
#define PWM_5_NAME "Royal Blue 3\0"
#define PWM_6_NAME "Blue\0"
#define PWM_7_NAME "Red\0"
#define PWM_8_NAME "Sump\0"

#define WATER_TEMPERATURE_MIN 24 // for coloring water temperature

// i2c configuration
#define PWM_I2C_MIN 0.0 // lower value of i2c scale
#define PWM_I2C_MAX 4095.0 // uper value
#define PWM_I2C_FREQ 333 // i2c frequency (hz)

// termometry
#define ONEWIRE_PIN 4 // ds18b20 thermometers pin

// if NEXTION_SOFTWARE_SERIAL defined, configure pins (for use with different pins - not stanard arduino Serial port)
#define NEXTION_SOFTWARE_PIN_TX A2
#define NEXTION_SOFTWARE_PIN_RX A1

// relay's pins
#define LED_FANS_PIN 2 // lamp fans relay pin
#define WATER_FANS_PIN 8 // water  fans relay pin
#define SUMP_FANS_PIN 7 // sump fans relay pin

// resolutions
#define PWM_RESOLUTION 500.0 //ms main PWM loop resolution
#define PWM_MIN_STEP 0.00001 // minimum pwm change step
#define NX_INFO_RESOLUTION 1000 //ms - nextion home page refresh time
#define EEPROM_STATE_RESOLUTION 5000 //ms - saving of valueCurrent value time
#define TEMPERATURE_SAMPLE_INTERVAL 1000 //ms temperature reading resolution
#define TIME_ADJUST_INTERVAL 900000 //s daylight saving mode check

// rozdzielczosc przekaznikow (s)
#define FANS_INTERVAL 300000 //ms fans resolution

#define MAX_WATTS 200 // RFU
