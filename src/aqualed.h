#ifndef AQUALED_H
#define AQUALED_H
/*
         AQUALED system configuration file (c) T. Formanowski 2016-2017
         https://github.com/mathompl/AquaLed
 */
#include <Wire.h>
#include <OneWire.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <DS18B20.h>
#include "RTClib.h"
#include "config.h"

/*
         SYSTEM VARIABLES, do not modify
         User configuration in file config.h
 */

#define ON true
#define OFF false

RTC_DS3231 RTC;

// sensors
#define LED_TEMPERATURE_FAN 0
#define SUMP_TEMPERATURE_FAN 1
#define WATER_TEMPERATURE_FAN 2

// structure for storing channel information
// do not modify order (written raw to eeprom)
typedef struct {
        byte enabled; // is channel enabled
        byte onHour; // channel daylight start hour
        byte onMinute; // channel daylight start minute
        byte useLunarPhase;
        byte offHour; // channel daylight stop hour
        byte offMinute; // channel daylight stop minute
        byte valueNight; // nightlight value
        int valueDay; // daylight value
        byte sunriseLenght; // minutes
        byte sunsetLenght; // minutes
        byte isNightLight; // is channel a nightlight
        int valueProg; // additional program value
        byte isProg;
        byte pin; // pwm pin
        byte isI2C; // use build in bin or i2c module pin
        byte invertPwm; // invert pwm values
        byte watts;
} PWM_SETTINGS;

// PWM channel runtime data
typedef struct {
        long startTime;
        long stopTime;
        long sunriseTime;
        long sunsetTime;
        bool dimmingStart;
        bool recoverLastState;
        double valueGoal;
        double valueCurrent;
        double pwmLast;
        double nxPwmLast;
        byte isSunrise;
        byte isSunset;
        byte isNight;
        bool testMode;
        long secondsLeft; // sunset/sunrise
        double step;
        double watts;
} PWM_RUNTIME;

// Settings
// do not modify order (written to eeprom)
typedef struct {
        byte forceNight;
        byte forceAmbient;
        byte forceOFF;
        byte maxTemperatures[3];
        byte pwmDimmingTime;
        byte screenSaverTime;
        byte softDimming;
        byte dst;
        byte sensors[7][8];
} SETTINGS;

PWM_SETTINGS pwmSettings[PWMS] = {0};
PWM_RUNTIME pwmRuntime[PWMS] = {0};
SETTINGS settings = {0};

typedef struct {
        byte address[8];
        boolean detected;
} SENSORSLIST;

SENSORSLIST sensorsList[7] = {0};

typedef struct {
        byte pin;
        float temperature;
        float nxTemperature;
        bool fanStatus;
        bool nxFanStatus;

} SENSORS;

SENSORS sensors[3] = {0};

// time variables
long currTime = 0;
uint8_t currHour = 0;
uint8_t currMinute = 0;
uint8_t currSecond = 0;
long unsigned currentMillis = 0;
unsigned long previousRTCCall = 0;
unsigned long previousPwmResolution = 0;
unsigned long previousNxInfo = 0;
unsigned long previousTemperature = 0;
unsigned long previousMillisFans = 0;
unsigned long previousMillisNextion = 0;
unsigned long previousSecTimeAdjust = 0;
unsigned long lastTouch = 0;
uint32_t startTimestamp = 0;

// aux
bool lampOverheating = false;
float watts = 0;
bool max_watts_exceeded = false;
byte codePoint = 0;

// moonphases
byte moonPhase = 0;
const byte moonPhases[] = {0,  1,  4,  9,  16,  25, 36, 50, 58, 69,
                           79, 88, 94, 99, 100, 99, 95, 90, 83, 75,
                           66, 57, 50, 38, 29,  21, 13, 7,  3,  1};

// logarithmic dimming table (dont use with I2C module)
// const byte dimmingTable [] PROGMEM = {
#ifndef NO_DIMMING_TABLE
const byte dimmingTable[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04,
        0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07,
        0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A,
        0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F,
        0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x14, 0x14, 0x15, 0x16,
        0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1C, 0x1C, 0x1D,
        0x1E, 0x1E, 0x1F, 0x20, 0x21, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26,
        0x27, 0x28, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2E, 0x2F, 0x30,
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
        0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4B, 0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57, 0x59,
        0x5A, 0x5B, 0x5D, 0x5E, 0x5F, 0x61, 0x62, 0x63, 0x65, 0x66, 0x68, 0x69,
        0x6B, 0x6C, 0x6E, 0x6F, 0x71, 0x72, 0x74, 0x75, 0x77, 0x79, 0x7A, 0x7C,
        0x7D, 0x7F, 0x81, 0x82, 0x84, 0x86, 0x87, 0x89, 0x8B, 0x8D, 0x8E, 0x90,
        0x92, 0x94, 0x96, 0x97, 0x99, 0x9B, 0x9D, 0x9F, 0xA1, 0xA3, 0xA5, 0xA6,
        0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBD, 0xBF,
        0xC1, 0xC3, 0xC5, 0xC7, 0xC9, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD7, 0xD9,
        0xDB, 0xDD, 0xE0, 0xE2, 0xE4, 0xE7, 0xE9, 0xEB, 0xEE, 0xF0, 0xF3, 0xF5,
        0xF8, 0xFA, 0xFD, 0xFF,
};
#endif


#endif
