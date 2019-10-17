

#define PWMS 6
#define PWM1_PIN  3
#define PWM2_PIN  5
#define PWM3_PIN  6
#define PWM4_PIN  9
#define PWM5_PIN  10
#define PWM6_PIN  11

#define ONEWIRE_PIN 4
#define FANS_PIN 2
#define AQUA_PIN 7

#define PWM_DIMMING_TIME 30000
#define PWM_RESOLUTION 100
#define NX_INFO_RESOLUTION 2000
#define AMBIENT_VALUE  25


#define MAX_BULB_TEMP 35
#define MAX_AQUA_TEMP 30
#define FANS_INTERVAL 300
#define AQUA_INTERVAL 300

#define ON  true
#define OFF  false


typedef struct 
{
     byte pwmPin;
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
     boolean pwmInvert;     
     double pwmNow;
     byte pwmLast;
     byte pwmGoal;
} PWM;

PWM pwm_list[6];

boolean forceNight = false;
boolean forceAmbient = false;
boolean forceOFF = false;


unsigned long previousPwmResolution = 0;
unsigned long previousNxInfo = 0;
unsigned long previousMillisFans = 0;
unsigned long previousMillisAqua = 0;
unsigned long previousMillisNextion = 0;

long unsigned currentMillis;
long unsigned currentTimeSec;

tmElements_t tm;

char cmdOutputArray[64];

boolean justTurnedOn = true;


// sensors
byte sensorAddressFans[8] = {0x28, 0x32, 0x78, 0x04, 0x00, 0x00, 0x80, 0x4C};
byte sensorAddressAqua[8] = {0x28, 0x12, 0x50, 0x28, 0x00, 0x00, 0x80, 0x5E};


float temperatureFans;
float temperatureAqua;

float nxtemperatureFans;
float nxtemperatureAqua;

boolean ledFansStatus = false;
boolean waterFansStatus = false;

boolean nxledFansStatus = true;
boolean nxwaterFansStatus = true;

byte nxScreen = 0;

byte nxLastHour=0, nxLastMinute=0;



















