/**************************************************************
   AQUALED PWM library header file (c) T. Formanowski 2016-2022
   https://github.com/mathompl/AquaLed
   GNU GENERAL PUBLIC LICENSE
**************************************************************/

#ifndef PWM_H
#define PWM_H

#include "config.h"
#include "time.h"
#include <Arduino.h>

#ifndef USE_ADAFRUIT_LIBRARY
#include "PCA9685.h"
PCA9685 pwmController(B000000);
#define PWM_I2C_SETVALUE(x, y) pwmController.setChannelPWM(x, y)
#else
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm_i2c = Adafruit_PWMServoDriver();
#define PWM_I2C_SETVALUE(x, y) pwm_i2c.setPWM(x, 0, y)
#endif

typedef struct {
        long startTime;
        long stopTime;
        long sunriseTime;
        long sunsetTime;
        bool dimmingStarted;
        bool recoverLastState;
        double valueGoal;
        double valueCurrent;
        double valueLast;
        double valueLastNextion;
        byte isSunrise;
        byte isSunset;
        byte isNight;
        bool testMode;
        long secondsLeft; // sunset/sunrise
        double step;
        double watts;
} PWM_RUNTIME;

class PWM {
public:
    PWM(_Time *_time);
    void begin();
    void initPWM(byte pwmId);
    void recoverSunsetAndSunrise(byte pwmId);
    void resetFlags(byte pwmId);
    void forcePWMRecovery(boolean test);
    void forceDimmingRestart();
    void loop();
    PWM_RUNTIME getRuntime(int pwm);
    void updateChannelTimes(byte pwmId);
    void setTestMode (byte pwm, bool value);
    void setValueLastNextion (byte pwm, double value);
    void setCurrentValue(byte pwm, double value);
    long getSunsetSeconds(byte pwmId);
    long getSunriseSeconds(byte pwmId);
    long mapRound(long x, long in_min,long in_max, long out_min, long out_max);
    double mapDouble(double x, double in_min, double in_max, double out_min, double out_max);

private:
    PWM_RUNTIME __pwmRuntime[PWMS] = {0};
    _Time *__time;
    void pwmStep(byte pwmId);
    void initDimming(byte pwmId, double dimmingScale, long dimmingTime);
    void goalReached(byte pwmId);
    double getNightValue(byte pwmId);
    bool getState(byte pwmId);
    void loop_channel(byte pwmId);
};

#endif
