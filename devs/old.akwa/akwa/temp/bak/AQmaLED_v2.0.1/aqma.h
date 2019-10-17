// AQUMA
boolean ON = true, OFF = false; ///
boolean pwm1Invert = false; ///
boolean pwm2Invert = false; ///
boolean pwm3Invert = false; ///
boolean pwm4Invert = false; ///
boolean pwm5Invert = false; ///
boolean pwm6Invert = false; ///
byte pwm1Pin = 3; ///
byte pwm2Pin = 5; ///
byte pwm3Pin = 6; ///
byte pwm4Pin = 9; ///
byte pwm5Pin = 10; ///
byte pwm6Pin = 11; ///
int pwmSilkySmootTimeSec = 30; ///
tmElements_t tm;
long unsigned currentTimeSec;
char cmdOutputArray[64];

// PWM - 1
byte pwm1Status, pwm1HOn, pwm1MOn, pwm1SOn;
byte pwm1HOff, pwm1MOff, pwm1SOff, pwm1Min, pwm1Max;
byte pwm1Sr, pwm1Ss, pwm1KeepLight;

// PWM - 2
byte pwm2Status, pwm2HOn, pwm2MOn, pwm2SOn;
byte pwm2HOff, pwm2MOff, pwm2SOff, pwm2Min, pwm2Max;
byte pwm2Sr, pwm2Ss, pwm2KeepLight;

// PWM - 3
byte pwm3Status, pwm3HOn, pwm3MOn, pwm3SOn;
byte pwm3HOff, pwm3MOff, pwm3SOff, pwm3Min, pwm3Max;
byte pwm3Sr, pwm3Ss, pwm3KeepLight;

// PWM - 4
byte pwm4Status, pwm4HOn, pwm4MOn, pwm4SOn;
byte pwm4HOff, pwm4MOff, pwm4SOff, pwm4Min, pwm4Max;
byte pwm4Sr, pwm4Ss, pwm4KeepLight;

// PWM - 5
byte pwm5Status, pwm5HOn, pwm5MOn, pwm5SOn;
byte pwm5HOff, pwm5MOff, pwm5SOff, pwm5Min, pwm5Max;
byte pwm5Sr, pwm5Ss, pwm5KeepLight;

// PWM - 6
byte pwm6Status, pwm6HOn, pwm6MOn, pwm6SOn;
byte pwm6HOff, pwm6MOff, pwm6SOff, pwm6Min, pwm6Max;
byte pwm6Sr, pwm6Ss, pwm6KeepLight;

boolean justTurnedOn = true;

int i_id = 0;




// MATHOM
#define MAX_BULB_TEMP 35
#define MAX_AQUA_TEMP 30
#define FANS_INTERVAL 300
#define AQUA_INTERVAL 300
#define NEXTION_UPDATE_INTERVAL 30
const byte ONEWIRE_PIN = 4;
const byte FANS_PIN = 2;
const byte AQUA_PIN = 7;

// sensors
byte sensorAddressFans[8] = {0x28, 0x32, 0x78, 0x04, 0x00, 0x00, 0x80, 0x4C};
byte sensorAddressAqua[8] = {0x28, 0x12, 0x50, 0x28, 0x00, 0x00, 0x80, 0x5E};


float temperatureFans;
float temperatureAqua;

float nxtemperatureFans;
float nxtemperatureAqua;


unsigned long previousMillisFans = 0;
unsigned long previousMillisAqua = 0;
unsigned long previousMillisNextion = 0;

boolean forceNight = false;
boolean forceAmbient = false;
boolean forceOFF = false;
#define AmbientPercent  10

byte led1status;
byte led2status;
byte led3status;
byte led4status;
byte led5status;
byte led6status;

byte led1statuspercent;
byte led2statuspercent;
byte led3statuspercent;
byte led4statuspercent;
byte led5statuspercent;
byte led6statuspercent;

byte nxled1statuspercent=-1;
byte nxled2statuspercent=-1;
byte nxled3statuspercent=-1;
byte nxled4statuspercent=-1;
byte nxled5statuspercent=-1;
byte nxled6statuspercent=-1;

boolean ledFansStatus = false;
boolean waterFansStatus = false;

boolean nxledFansStatus = true;
boolean nxwaterFansStatus = true;

byte nxScreen = 0;

byte nxLastHour=0, nxLastMinute=0;
