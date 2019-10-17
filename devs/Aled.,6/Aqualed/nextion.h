/*
   AQUALED Nextion configuration file (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#include <Arduino.h>
#include <avr/pgmspace.h>

#ifdef NEXTION_SOFTWARE_SERIAL
#include "SoftwareSerial.h"
SoftwareSerial nx(NEXTION_SOFTWARE_PIN_TX, NEXTION_SOFTWARE_PIN_RX);
#define NEXTION_BEGIN(x) nx.begin(x)
#define NEXTION_READ(x) nx.read()
#define NEXTION_FLUSH(x) nx.flush()
#define NEXTION_READBYTES(x, y) nx.readBytes(x, y)
#define NEXTION_SETTIMEOUT(x) nx.setTimeout(x)
#define NEXTION_AVAIL() nx.available()
#define NEXTION_PRINT(x) nx.print(x)
#define NEXTION_PRINTF(x, y) nx.print(x, y)
#define NEXTION_WRITE(x) nx.write(x)
#define NEXTION_WRITEB(x, y) nx.write(x, y)
#define NEXTION_END() nx.end();
#else
#define NEXTION_BEGIN(x) Serial.begin(x)
#define NEXTION_FLUSH(x) Serial.flush()
#define NEXTION_READ(x) Serial.read()
#define NEXTION_READBYTES(x, y) Serial.readBytes(x, y)
#define NEXTION_SETTIMEOUT(x) Serial.setTimeout(x)
#define NEXTION_AVAIL() Serial.available()
#define NEXTION_PRINT(x) Serial.print(x)
#define NEXTION_PRINTF(x, y) Serial.print(x, y)
#define NEXTION_WRITE(x) Serial.write(x)
#define NEXTION_WRITEB(x, y) Serial.write(x, y)
#define NEXTION_END() Serial.end();
#endif

// harmonogram / schedule
#define min_hour 1440
#define offset 21
#define width 16
#define height 240
#define startx 47
#define starty 31
#define hour_startx 40
#define hour_stopx 180

#define DEFAULT_BAUD_RATE 9600
#define NEXTION_BAUD_RATE 115200
//#define NEXTION_BAUD_RATE 115200

// uncomment for use nextion editor simulator
// needs additional settings for non-standard baud rates
//#define NEXTION_SIMULATOR 1

// nextion protocol
#define NEX_RET_CMD_FINISHED (0x01)
#define NEX_RET_EVENT_TOUCH_HEAD (0x65)
#define NEX_EVENT_POP (0x00)
#define NEX_EVENT_PUSH (0x01)
#define NEX_RET_STRING_HEAD (0x70)
#define NEX_RET_NUMBER_HEAD (0x71)

int t = 0;
bool forceRefresh = false;
byte nxScreen = 0;
byte time_separator = 0;
float lastWatts = 0;
const byte nextionEol[] = {0xFF, 0xFF, 0xFF};
byte lastH = 0, lastM = 0;
byte __touch_buffer_ix = 0;
boolean __touch_event = false;
byte __touch_buffer[7];
byte activePwmStatus;

// pages
#define PAGE_HOME 0
#define PAGE_CONFIG 1
#define PAGE_SETTIME 2
#define PAGE_SETTINGS 3
#define PAGE_PWM 4
#define PAGE_TEST 5
#define PAGE_SCREENSAVER 6
#define PAGE_THERMO 7
#define PAGE_SCHEDULE 8
#define PAGE_PWM_LIST 9
#define PAGE_ERROR 10
#define PAGE_SAVING 11
#define PAGE_PWMSTATUS 12

// buttons
#define THERMOPAGE_BUTTON_SAVE 9
#define THERMOPAGE_BUTTON_CANCEL 10
#define SCHEDULEPAGE_BUTTON_CLOSE 1
#define TESTPAGE_BUTTON_CLOSE 2
#define TESTPAGE_SLIDER_PWM_1 3
#define TESTPAGE_SLIDER_PWM_2 4
#define TESTPAGE_SLIDER_PWM_3 5
#define TESTPAGE_SLIDER_PWM_4 6
#define TESTPAGE_SLIDER_PWM_5 7
#define TESTPAGE_SLIDER_PWM_6 8
#define TESTPAGE_SLIDER_PWM_7 21
#define TESTPAGE_SLIDER_PWM_8 24
#define TESTPAGE_SLIDER_PWM_8 24
#define PWMPAGE_BUTTON_SAVE 13
#define PWMPAGE_BUTTON_CANCEL 14
#define SETTINGSPAGE_BUTTON_SAVE 8
#define SETTINGSPAGE_BUTTON_CANCEL 9
#define TIMEPAGE_BUTTON_SAVE 4
#define TIMEPAGE_BUTTON_CANCEL 5
#define CONFIGPAGE_BUTTON_PWM 1
#define CONFIGPAGE_BUTTON_SETTINGS 4
#define CONFIGPAGE_BUTTON_THERMO 7
#define CONFIGPAGE_BUTTON_TIME 2
#define CONFIGPAGE_BUTTON_TEST 6
#define CONFIGPAGE_BUTTON_SCHEDULE 8
#define CONFIGPAGE_BUTTON_CLOSE 5
#define PWMCONFIGPAGE_BUTTON_PWM_1 1
#define PWMCONFIGPAGE_BUTTON_PWM_2 2
#define PWMCONFIGPAGE_BUTTON_PWM_3 3
#define PWMCONFIGPAGE_BUTTON_PWM_4 4
#define PWMCONFIGPAGE_BUTTON_PWM_5 5
#define PWMCONFIGPAGE_BUTTON_PWM_6 6
#define PWMCONFIGPAGE_BUTTON_PWM_7 7
#define PWMCONFIGPAGE_BUTTON_PWM_8 8
#define PWMCONFIGPAGE_BUTTON_CLOSE 10
#define HOMEPAGE_BUTTON_OFF 22
#define HOMEPAGE_BUTTON_NIGHT 23
#define HOMEPAGE_BUTTON_AMBIENT 25
#define HOMEPAGE_BUTTON_CONFIG 24
#define HOMEPAGE_BUTTON_FAN_WATER 28
#define HOMEPAGE_BUTTON_FAN_LAMP 29
#define HOMEPAGE_BUTTON_FAN_SUMP 30
#define HOMEPAGE_PWMSTATUS1 31
#define HOMEPAGE_PWMSTATUS2 32
#define HOMEPAGE_PWMSTATUS3 33
#define HOMEPAGE_PWMSTATUS4 34
#define HOMEPAGE_PWMSTATUS5 35
#define HOMEPAGE_PWMSTATUS6 36
#define HOMEPAGE_PWMSTATUS7 37
#define HOMEPAGE_PWMSTATUS8 38
#define PWMSTATUSPAGE_BUTTON_CLOSE 1
#define COLOR_RED 32768
#define COLOR_GREEN 1024
#define COLOR_LIGHTRED 62225
#define COLOR_LIGHTGREEN 38898
#define COLOR_ORANGE 62945
#define COLOR_YELLOW 65504
#define COLOR_BLUE 34815
#define COLOR_DARKBLUE 16
#define COLOR_DARKGRAY 12678
#define COLOR_WHITE 65535
#define COLOR_LIGHTYELLOW 65531

#define NX_CMD_COMMA 0
#define NX_CMD_PARENTH 1
#define NX_CMD_SPACE 2
#define NX_CMD_EQ 3
#define NX_CMD_TXT 4
#define NX_CMD_PIC 5
#define NX_CMD_PCO 6
#define NX_CMD_GET 7
#define NX_CMD_VAL 8
#define NX_CMD_VIS 9
#define NX_CMD_PAGE 10
#define NX_CMD_FILL 11
#define NX_CMD_DOT 12
#define NX_CMD_EMPTY 255

#define NX_STR_DEGREE 0
#define NX_STR_ON 1
#define NX_STR_OFF 2
#define NX_STR_NIGHT 3
#define NX_STR_SUNRISE 4
#define NX_STR_SUNSET 5
#define NX_STR_DASH 6
#define NX_STR_SLASH 7
#define NX_STR_EMPTY 8
#define NX_STR_RECOVER 9
#define NX_STR_FAN 10
#define NX_STR_UP 11
#define NX_STR_DOWN 12
#define NX_STR_CELC 13
#define NX_STR_PERCENT 14
#define NX_STR_WATTS 15
#define NX_STR_SPACE 16

#define NX_FIELD_T0 0
#define NX_FIELD_T1 1
#define NX_FIELD_T2 2
#define NX_FIELD_T3 3
#define NX_FIELD_T4 4
#define NX_FIELD_T5 5
#define NX_FIELD_T6 6
#define NX_FIELD_T7 7
#define NX_FIELD_T8 8
#define NX_FIELD_T9 9
#define NX_FIELD_T10 10
#define NX_FIELD_P0 11
#define NX_FIELD_P1 12
#define NX_FIELD_P2 13
#define NX_FIELD_VA0 14
#define NX_FIELD_VA1 15
#define NX_FIELD_VA2 16
#define NX_FIELD_VA3 17
#define NX_FIELD_C0 18
#define NX_FIELD_C1 19
#define NX_FIELD_C2 20
#define NX_FIELD_C3 21
#define NX_FIELD_C4 22
#define NX_FIELD_C5 23
#define NX_FIELD_C6 24
#define NX_FIELD_C7 25
#define NX_FIELD_C8 26
#define NX_FIELD_C9 27
#define NX_FIELD_C10 28
#define NX_FIELD_N0 29
#define NX_FIELD_N1 30
#define NX_FIELD_N2 31
#define NX_FIELD_N3 32
#define NX_FIELD_N4 33
#define NX_FIELD_N5 34
#define NX_FIELD_N6 35
#define NX_FIELD_N7 36
#define NX_FIELD_N8 37
#define NX_FIELD_N9 38
#define NX_FIELD_N10 39
#define NX_FIELD_WT 40
#define NX_FIELD_LT 41
#define NX_FIELD_ST 42
#define NX_FIELD_TI 43
#define NX_FIELD_H 44
#define NX_FIELD_BO 45
#define NX_FIELD_BA 46
#define NX_FIELD_BN 47
#define NX_FIELD_LD1 48
#define NX_FIELD_LD2 49
#define NX_FIELD_LD3 50
#define NX_FIELD_LD4 51
#define NX_FIELD_LD5 52
#define NX_FIELD_LD6 53
#define NX_FIELD_LD7 54
#define NX_FIELD_LD8 55
#define NX_FIELD_BLD1 56
#define NX_FIELD_BLD2 57
#define NX_FIELD_BLD3 58
#define NX_FIELD_BLD4 59
#define NX_FIELD_BLD5 60
#define NX_FIELD_BLD6 61
#define NX_FIELD_BLD7 62
#define NX_FIELD_BLD8 63
#define NX_FIELD_BAUDS 64
#define NX_FIELD_BKCMD 65
#define NX_FIELD_L1 66
#define NX_FIELD_L2 67
#define NX_FIELD_L3 68
#define NX_FIELD_L4 69
#define NX_FIELD_L5 70
#define NX_FIELD_L6 71
#define NX_FIELD_L7 72
#define NX_FIELD_L8 73
#define NX_FIELD_N11 74
#define NX_FIELD_WA 75
#define NX_FIELD_EMPTY 255

#define NX_PIC_BO_OFF 3
#define NX_PIC_BO_ON 2
#define NX_PIC_BA_OFF 6
#define NX_PIC_BA_ON 7
#define NX_PIC_BN_OFF 4
#define NX_PIC_BN_ON 5

// nextion fields
const char f_t0[] PROGMEM = "t0";
const char f_t1[] PROGMEM = "t1";
const char f_t2[] PROGMEM = "t2";
const char f_t3[] PROGMEM = "t3";
const char f_t4[] PROGMEM = "t4";
const char f_t5[] PROGMEM = "t5";
const char f_t6[] PROGMEM = "t6";
const char f_t7[] PROGMEM = "t7";
const char f_t8[] PROGMEM = "t8";
const char f_t9[] PROGMEM = "t9";
const char f_t10[] PROGMEM = "t10";
const char f_p0[] PROGMEM = "p0";
const char f_p1[] PROGMEM = "p1";
const char f_p2[] PROGMEM = "p2";
const char f_va0[] PROGMEM = "va0";
const char f_va1[] PROGMEM = "va1";
const char f_va2[] PROGMEM = "va2";
const char f_va3[] PROGMEM = "va3";
const char f_c0[] PROGMEM = "c0";
const char f_c1[] PROGMEM = "c1";
const char f_c2[] PROGMEM = "c2";
const char f_c3[] PROGMEM = "c3";
const char f_c4[] PROGMEM = "c4";
const char f_c5[] PROGMEM = "c5";
const char f_c6[] PROGMEM = "c6";
const char f_c7[] PROGMEM = "c7";
const char f_c8[] PROGMEM = "c8";
const char f_c9[] PROGMEM = "c9";
const char f_c10[] PROGMEM = "c10";
const char f_n0[] PROGMEM = "n0";
const char f_n1[] PROGMEM = "n1";
const char f_n2[] PROGMEM = "n2";
const char f_n3[] PROGMEM = "n3";
const char f_n4[] PROGMEM = "n4";
const char f_n5[] PROGMEM = "n5";
const char f_n6[] PROGMEM = "n6";
const char f_n7[] PROGMEM = "n7";
const char f_n8[] PROGMEM = "n8";
const char f_n9[] PROGMEM = "n9";
const char f_n10[] PROGMEM = "n10";
const char f_n11[] PROGMEM = "n11";
const char f_wt[] PROGMEM = "wt";
const char f_lt[] PROGMEM = "lt";
const char f_st[] PROGMEM = "st";
const char f_ti[] PROGMEM = "ti";
const char f_h[] PROGMEM = "hour";
const char f_bo[] PROGMEM = "bo";
const char f_ba[] PROGMEM = "ba";
const char f_bn[] PROGMEM = "bn";
const char f_ld1[] PROGMEM = "ld1";
const char f_ld2[] PROGMEM = "ld2";
const char f_ld3[] PROGMEM = "ld3";
const char f_ld4[] PROGMEM = "ld4";
const char f_ld5[] PROGMEM = "ld5";
const char f_ld6[] PROGMEM = "ld6";
const char f_ld7[] PROGMEM = "ld7";
const char f_ld8[] PROGMEM = "ld8";
const char f_bld1[] PROGMEM = "bld1";
const char f_bld2[] PROGMEM = "bld2";
const char f_bld3[] PROGMEM = "bld3";
const char f_bld4[] PROGMEM = "bld4";
const char f_bld5[] PROGMEM = "bld5";
const char f_bld6[] PROGMEM = "bld6";
const char f_bld7[] PROGMEM = "bld7";
const char f_bld8[] PROGMEM = "bld8";
const char f_bauds[] PROGMEM = "baud";
const char f_bkcmd[] PROGMEM = "bkcmd";
const char f_l1[] PROGMEM = "l1";
const char f_l2[] PROGMEM = "l2";
const char f_l3[] PROGMEM = "l3";
const char f_l4[] PROGMEM = "l4";
const char f_l5[] PROGMEM = "l5";
const char f_l6[] PROGMEM = "l6";
const char f_l7[] PROGMEM = "l7";
const char f_l8[] PROGMEM = "l8";
const char f_wa[] PROGMEM = "wa";

#define PAGE_HOME 0
#define PAGE_CONFIG 1
#define PAGE_SETTIME 2
#define PAGE_SETTINGS 3
#define PAGE_PWM 4
#define PAGE_TEST 5
#define PAGE_SCREENSAVER 6
#define PAGE_THERMO 7
#define PAGE_SCHEDULE 8
#define PAGE_PWM_LIST 9
#define PAGE_ERROR 10
#define PAGE_SAVING 11

const char p_home[] PROGMEM = "home";
const char p_config[] PROGMEM = "";
const char p_settime[] PROGMEM = "time";
const char p_settings[] PROGMEM = "settings";
const char p_pwm[] PROGMEM = "pwm";
const char p_test[] PROGMEM = "";
const char p_screensaver[] PROGMEM = "";
const char p_thermo[] PROGMEM = "thermo";
const char p_schedule[] PROGMEM = "";
const char p_pwmlist[] PROGMEM = "pwmconfig";
const char p_error[] PROGMEM = "";
const char p_saving[] PROGMEM = "";

// nextion commands
const char cmd_comma[] PROGMEM = ",";
const char cmd_parenth[] PROGMEM = "\"";
const char cmd_space[] PROGMEM = " ";
const char cmd_eq[] PROGMEM = "=";
const char cmd_txt[] PROGMEM = ".txt";
const char cmd_pic[] PROGMEM = ".pic";
const char cmd_pco[] PROGMEM = ".pco";
const char cmd_get[] PROGMEM = "get";
const char cmd_val[] PROGMEM = ".val";
const char cmd_vis[] PROGMEM = "vis";
const char cmd_page[] PROGMEM = "page";
const char cmd_fill[] PROGMEM = "fill";
const char cmd_dot[] PROGMEM = ".";

// constant strings
const char str_degree[] PROGMEM = "EC";
const char str_on[] PROGMEM = "A";
const char str_off[] PROGMEM = "S";
const char str_night[] PROGMEM = "P";
const char str_sunrise[] PROGMEM = "G";
const char str_sunset[] PROGMEM = "H";
const char str_dash[] PROGMEM = "-";
const char str_slash[] PROGMEM = "/";
const char str_empty[] PROGMEM = "";
const char str_recover[] PROGMEM = "D";
const char str_fan[] PROGMEM = "U";
const char str_up[] PROGMEM = "L";
const char str_down[] PROGMEM = "R";
const char str_celc[] PROGMEM = "'C";
const char str_percent[] PROGMEM = "%";
const char str_watts[] PROGMEM = "W";
const char str_space[] PROGMEM = " ";

// nextion error descriptions
const char str_er1[] PROGMEM = "OVERHEAT";
const char str_er2[] PROGMEM = "MAXPOWER";

char const pwm_1_name[] PROGMEM = PWM_1_NAME;
char const pwm_2_name[] PROGMEM = PWM_2_NAME;
char const pwm_3_name[] PROGMEM = PWM_3_NAME;
char const pwm_4_name[] PROGMEM = PWM_4_NAME;
char const pwm_5_name[] PROGMEM = PWM_5_NAME;
char const pwm_6_name[] PROGMEM = PWM_6_NAME;
char const pwm_7_name[] PROGMEM = PWM_7_NAME;
char const pwm_8_name[] PROGMEM = PWM_8_NAME;

const char *const nx_pwm_names[] PROGMEM{pwm_1_name, pwm_2_name, pwm_3_name,
                                         pwm_4_name, pwm_5_name, pwm_6_name,
                                         pwm_7_name, pwm_8_name};
const char *const nx_strings[] PROGMEM{
    str_degree, str_on,    str_off,     str_night,   str_sunrise, str_sunset,
    str_dash,   str_slash, str_empty,   str_recover, str_fan,     str_up,
    str_down,   str_celc,  str_percent, str_watts,   str_space};
const char *const nx_commands[] PROGMEM{
    cmd_comma, cmd_parenth, cmd_space, cmd_eq,   cmd_txt,  cmd_pic, cmd_pco,
    cmd_get,   cmd_val,     cmd_vis,   cmd_page, cmd_fill, cmd_dot};
const char *const nx_fields[] PROGMEM{
    f_t0,   f_t1,    f_t2,    f_t3,   f_t4,   f_t5,   f_t6,   f_t7,   f_t8,
    f_t9,   f_t10,   f_p0,    f_p1,   f_p2,   f_va0,  f_va1,  f_va2,  f_va3,
    f_c0,   f_c1,    f_c2,    f_c3,   f_c4,   f_c5,   f_c6,   f_c7,   f_c8,
    f_c9,   f_c10,   f_n0,    f_n1,   f_n2,   f_n3,   f_n4,   f_n5,   f_n6,
    f_n7,   f_n8,    f_n9,    f_n10,  f_wt,   f_lt,   f_st,   f_ti,   f_h,
    f_bo,   f_ba,    f_bn,    f_ld1,  f_ld2,  f_ld3,  f_ld4,  f_ld5,  f_ld6,
    f_ld7,  f_ld8,   f_bld1,  f_bld2, f_bld3, f_bld4, f_bld5, f_bld6, f_bld7,
    f_bld8, f_bauds, f_bkcmd, f_l1,   f_l2,   f_l3,   f_l4,   f_l5,   f_l6,
    f_l7,   f_l8,    f_n11,   f_wa};
const char *const nx_errors[] PROGMEM{str_er1, str_er2};
const char *const nx_pages[] PROGMEM{
    p_home,        p_config, p_settime,  p_settings, p_pwm,   p_test,
    p_screensaver, p_thermo, p_schedule, p_pwmlist,  p_error, p_saving};
