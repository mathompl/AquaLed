#include <Wire.h>
#include <Time.h>
#include <EEPROM.h>
#include <DS1307RTC.h>
#include <OneWire.h>
#include <avr/wdt.h>
#include <DallasTemperature.h>
#include <DS18B20.h>


/* Configuration file */

// uncomment to use I2C PWM MODULE, PINS CONFIGURATION FOLLOWS
//#define USE_I2C_PWM_MODULE

// modules, uncomment to disable, comment to enable
#define NO_BLUETOOTH
//#define NO_NEXTION
//#define NO_I2C
//#define NO_TEMPERATURE
//#define DEBUG

// ARDUINO build-in PWM pins config
#ifdef USE_I2C_PWM_MODULE
    #define PWMS 8
    #define PWM1_PIN  0
    #define PWM2_PIN  1
    #define PWM3_PIN  2
    #define PWM4_PIN  3
    #define PWM5_PIN  4
    #define PWM6_PIN  5
    #define PWM7_PIN  6
    #define PWM8_PIN  7

    #define PWM1_I2C  1
    #define PWM2_I2C  1
    #define PWM3_I2C  1
    #define PWM4_I2C  1
    #define PWM5_I2C  1
    #define PWM6_I2C  1
    #define PWM7_I2C  1
    #define PWM8_I2C  1
#else
// PWM MODULE pins config
    #define PWMS 8
    #define PWM1_PIN  3
    #define PWM2_PIN  5
    #define PWM3_PIN  6
    #define PWM4_PIN  9
    #define PWM5_PIN  10
    #define PWM6_PIN  11
    #define PWM7_PIN  1
    #define PWM8_PIN  2

    #define PWM1_I2C  0
    #define PWM2_I2C  0
    #define PWM3_I2C  0
    #define PWM4_I2C  0
    #define PWM5_I2C  0
    #define PWM6_I2C  0
    #define PWM7_I2C  1
    #define PWM8_I2C  1
#endif

#define PWM_MAX 255

// overwrite night mode values (when 1% resolution is too much)
//#define PWM_FORCE_NIGHT_VALUE 1

// i2c
#define PWM_I2C_MIN 0
#define PWM_I2C_MAX 4095
#define I2C_FREQ 333 //hz
byte i2c_invert=0;



// termometry
#define ONEWIRE_PIN 4

// relay's pins
#define LED_FANS_PIN 2
#define WATER_FANS_PIN 8
#define SUMP_FANS_PIN 7

// resolutions
#define PWM_RESOLUTION 500.0 //ms
#define PWM_MIN_STEP 0.00001
#define NX_INFO_RESOLUTION 1000 //ms
#define EEPROM_STATE_RESOLUTION 5000 //ms

// rozdzielczosc przekaznikow (s)
#define LED_FANS_INTERVAL 300 //s
#define WATER_FANS_INTERVAL 300 //s
#define SUMP_FANS_INTERVAL 300 //s

// probkowanie temperatury (ms)
#define TEMPERATURE_SAMPLE_INTERVAL 1000 //ms

// sprawdzeanie DST
#define TIME_ADJUST_INTERVAL 3600 //s

#define MAX_WATTS 200

#define ON  true
#define OFF  false

#define WATER_TEMPERATURE_MIN 24

typedef struct
{
        byte pwmPin;
        bool pwmI2C;
        byte pwmStatus;
        byte pwmHOn;
        byte pwmMOn;
        byte pwmSOn;
        byte pwmHOff;
        byte pwmMOff;
        byte pwmSOff;
        byte pwmMin;
        byte pwmMax;
        byte pwmSr;
        byte pwmSs;
        byte pwmKeepLight;
        byte pwmInvert;
        double pwmNow;
        byte pwmGoal;
        byte pwmSaved;
        byte pwmTest;
        byte isSunrise;
        byte isSunset;
        byte isNight;
        byte pwmAmbient;
        byte dimmingScale;
        bool dimmingStart;
        long dimmingTime;
        bool recoverLastState;
} PWM;
PWM pwmChannel[PWMS];

double pwmLast[PWMS] = {0};
double pwmNxLast[PWMS] = {0};

struct SETTINGS_STRUCT
{
        byte forceNight;
        byte forceAmbient;
        byte forceOFF;
        byte max_led_temp;
        byte max_water_temp;
        byte max_sump_temp;
        byte pwmDimmingTime;
        byte screenSaverTime;
        byte softDimming;
        byte dst;
        byte ledSensorAddress[8];
        byte sumpSensorAddress[8];
        byte waterSensorAddress[8];
};

byte sensorsList[7][8];
bool sensorsDetected[7];

SETTINGS_STRUCT SETTINGS =
{
        0,
        0,
        0,
        35,
        30,
        30,
        35,
        30,
        0,
        0,
        {0x28, 0x32, 0x78, 0x04, 0x00, 0x00, 0x80, 0x4C},
        {0x28, 0x12, 0x50, 0x28, 0x00, 0x00, 0x80, 0x5E},
        {0x28, 0xFF, 0xBE, 0xCD, 0x6D, 0x14, 0x04, 0x02}
};

unsigned long previousPwmResolution = 0;
unsigned long previousNxInfo = 0;
unsigned long previousTemperature = 0;
unsigned long previousMillisFans = 0;
unsigned long previousMillisWater = 0;
unsigned long previousMillisSump = 0;
unsigned long previousMillisNextion = 0;
unsigned long previousMillisEepromState = 0;
unsigned long previousSecTimeAdjust = 0;
unsigned long lastTouch = 0;

tmElements_t tm;
long unsigned currentMillis;
long unsigned currentTimeSec;
bool  testMode = false;
byte nxLastHour = 0, nxLastMinute = 0;
bool  justTurnedOn = true;

// sensors
float temperatureLed;
float temperatureWater;
float temperatureSump;
float nxtemperatureLed;
float nxtemperatureWater;
float nxtemperatureSump;

bool  ledFansStatus = false;
bool  waterFansStatus = false;
bool  sumpFansStatus = false;
bool  nxledFansStatus = false;
bool  nxwaterFansStatus = false;
bool  nxsumpFansStatus = false;

// logarithmic dimming table
const byte dimmingTable [] PROGMEM = {
//const byte dimmingTable [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04,
        0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06,
        0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08,
        0x08, 0x08, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A,
        0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E,
        0x0E, 0x0F, 0x0F, 0x0F, 0x10, 0x10, 0x11, 0x11,
        0x12, 0x12, 0x13, 0x13, 0x14, 0x14, 0x15, 0x16,
        0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1A, 0x1A,
        0x1B, 0x1C, 0x1C, 0x1D, 0x1E, 0x1E, 0x1F, 0x20,
        0x21, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26,
        0x27, 0x28, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
        0x2E, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
        0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
        0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x43, 0x44, 0x45,
        0x46, 0x47, 0x48, 0x49, 0x4B, 0x4C, 0x4D, 0x4E,
        0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57, 0x59,
        0x5A, 0x5B, 0x5D, 0x5E, 0x5F, 0x61, 0x62, 0x63,
        0x65, 0x66, 0x68, 0x69, 0x6B, 0x6C, 0x6E, 0x6F,
        0x71, 0x72, 0x74, 0x75, 0x77, 0x79, 0x7A, 0x7C,
        0x7D, 0x7F, 0x81, 0x82, 0x84, 0x86, 0x87, 0x89,
        0x8B, 0x8D, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x97,
        0x99, 0x9B, 0x9D, 0x9F, 0xA1, 0xA3, 0xA5, 0xA6,
        0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6,
        0xB8, 0xBA, 0xBD, 0xBF, 0xC1, 0xC3, 0xC5, 0xC7,
        0xC9, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD7, 0xD9,
        0xDB, 0xDD, 0xE0, 0xE2, 0xE4, 0xE7, 0xE9, 0xEB,
        0xEE, 0xF0, 0xF3, 0xF5, 0xF8, 0xFA, 0xFD, 0xFF,
};
