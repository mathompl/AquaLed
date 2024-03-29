#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "aqualed.h"
#include <avr/pgmspace.h>

// default settings to write to EEprom
const SETTINGS defaultSettings PROGMEM = {0,0,0,{0,0,0}, 30, 30, 0,0,
{
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
}
};

const PWM_SETTINGS defaultPWMSettings[] PROGMEM =
{
        {0,9,0,0,20,0,0,(int)(PWM_I2C_MAX*0.6),60,60,0,0,0,15,1,0,30}, //CW1
        {0,9,0,0,20,0,0,(int)(PWM_I2C_MAX*0.6),60,60,0,0,0,14,1,0,30}, //CW2
        {0,8,0,0,21,0,0,(int)(PWM_I2C_MAX*0.8),60,60,0,0,0,13,1,0,30}, //RB1
        {0,8,0,0,21,0,0,(int)(PWM_I2C_MAX*0.8),60,60,0,0,0,12,1,0,30}, //RB2
        {0,8,0,0,21,0,0,(int)(PWM_I2C_MAX*0.8),60,60,0,0,0,11,1,0,30}, //RB3
        {0,8,0,0,21,0,80,(int)(PWM_I2C_MAX*0.8),60,60,1,0,0,10,1,0,30}, //B1
        {0,8,0,0,21,0,0,(int)(PWM_I2C_MAX*0.8),60,60,0,0,0,9,1,0,30}, //UV1
        {0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //SUMP
};

#endif
