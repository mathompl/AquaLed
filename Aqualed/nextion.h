#include <Arduino.h>
#include <avr/pgmspace.h>

#define DEFAULT_BAUD_RATE 9600
#define NEXTION_BAUD_RATE 115200

// needed when hotplugging nextion when baud rate !=9600
#define NEXTION_REINIT_TIME 5000 //ms
unsigned long previousNxReinit= 0;

// uncomment for use nextion editor simulator
// needs additional settings for non-standard baud rates
//#define NEXTION_SIMULATOR 1

#define NEX_RET_CMD_FINISHED                (0x01)
#define NEX_RET_EVENT_TOUCH_HEAD            (0x65)
#define NEX_EVENT_POP                       (0x00)
#define NEX_EVENT_PUSH                      (0x01)
#define NEX_RET_STRING_HEAD                 (0x70)
#define NEX_RET_NUMBER_HEAD                 (0x71)

// harmonogram
#define min_hour 1440
#define offset 21
#define width  16
#define height 240
#define startx 47
#define starty 31
#define hour_startx 40
#define hour_stopx 180

char buffer[30] = {0};
int t;
char ti[1] = {0};
char b[10]  = {0};
bool forceRefresh = false;
uint8_t nxScreen = 0;

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

// commands
#define CMD_SET_HOUR  0
#define CMD_PARENTH     1
#define CMD_BO_SET_PIC  2
#define CMD_BO_SET_PIC2 3
#define CMD_BO_SET_ALT_PIC2 4
#define CMD_BO_SET_ALT_PIC 5
#define CMD_BO_REFRESH 6
#define CMD_BA_SET_PIC 7
#define CMD_BA_SET_PIC2 8
#define CMD_BA_SET_ALT_PIC2 9
#define CMD_BA_SET_ALT_PIC 10
#define CMD_BA_REFRESH 11
#define CMD_BN_SET_PIC 12
#define CMD_BN_SET_PIC2 13
#define CMD_BN_SET_ALT_PIC2 14
#define CMD_BN_SET_ALT_PIC 15
#define CMD_BN_REFRESH 16
#define CMD_SET_L1 17
#define CMD_SET_L2 18
#define CMD_SET_L3 19
#define CMD_SET_L4 20
#define CMD_SET_L5 21
#define CMD_SET_L6 22
#define CMD_SET_L7 23
#define CMD_SET_L8 24
#define CMD_SET_LT 25
#define CMD_SET_WT 26
#define CMD_INIT1 27
#define CMD_INIT2 28
#define CMD_INIT3 58
#define CMD_SET_PAGE 29
#define CMD_SET_T0 100
#define CMD_SET_T1 30
#define CMD_SET_T2 31
#define CMD_SET_T3 32
#define CMD_SET_T4 33
#define CMD_SET_T5  34
#define CMD_SET_T6  35
#define CMD_SET_T7  36
#define CMD_SET_T8  37
#define CMD_SET_T9  38
#define CMD_SET_T10 39
#define CMD_GET_T1 40
#define CMD_GET_T2 41
#define CMD_GET_T3 42
#define CMD_GET_T4 43
#define CMD_GET_T5 44
#define CMD_GET_T6 45
#define CMD_GET_T7 46
#define CMD_GET_T8 47
#define CMD_GET_T9 48
#define CMD_GET_T10 49
#define CMD_SET_C0 50
#define CMD_SET_C1 51
#define CMD_GET_C0 52
#define CMD_GET_C1 53
#define CMD_SET_TI 54
#define CMD_GET_TI 55
#define CMD_SET_WT_RED 56
#define CMD_SET_WT_GREEN 57
#define CMD_SET_ST 59
#define CMD_SET_N0 60
#define CMD_SET_N1 61
#define CMD_SET_N2 62
#define CMD_SET_N3 63
#define CMD_SET_N4 64
#define CMD_SET_N5 65
#define CMD_SET_N6 66
#define CMD_SET_N7 67
#define CMD_SET_N8 68
#define CMD_SET_N9 69
#define CMD_GET_N0 70
#define CMD_GET_N1 71
#define CMD_GET_N2 72
#define CMD_GET_N3 73
#define CMD_GET_N4 74
#define CMD_GET_N5 75
#define CMD_GET_N6 76
#define CMD_GET_N7 77
#define CMD_GET_N8 78
#define CMD_GET_N9 79
//#define CMD_SET_C1 80
#define CMD_SET_C2 81
#define CMD_SET_C3 82
#define CMD_SET_C4 83
#define CMD_SET_C5 84
#define CMD_SET_C6 85
#define CMD_SET_C7 92
#define CMD_SET_C8 93
#define CMD_SET_VA0 86
#define CMD_SET_VA1 87
#define CMD_SET_VA2 88
#define CMD_SET_VA3 89
#define CMD_FILL 90
#define CMD_COMMA 91
#define CMD_SHOW_P0 94
#define CMD_SHOW_P1 95
#define CMD_SHOW_P2 96
#define CMD_HIDE_P0 97
#define CMD_HIDE_P1 98
#define CMD_HIDE_P2 99
#define CMD_SET_LT_RED 101
#define CMD_SET_LT_GREEN 102
#define CMD_SET_ST_RED 103
#define CMD_SET_ST_GREEN 104



#define STR_ON 0
#define STR_OFF 1
#define STR_NIGHT 2
#define STR_SUNRISE 3
#define STR_SUNSET 4
#define STR_DASH 5
#define STR_SLASH 6
#define STR_EMPTY 7
#define STR_RECOVER 8
#define STR_FAN 9
#define STR_UP 10
#define STR_DOWN 11
#define STR_SPACE 12

#define COLOR_RED 32768
#define COLOR_GREEN 1024
#define COLOR_ORANGE 33280
#define COLOR_YELLOW 65504
#define COLOR_BLUE 34815
#define COLOR_DARKBLUE 16
#define COLOR_DARKGRAY 12678

char const xbopic[] PROGMEM = "bo.pic=3";
char const xbopic2[] PROGMEM = "bo.pic=3";
char const xbo2pic[] PROGMEM = "bo.pic=2";
char const xbo2pic2[] PROGMEM = "bo.pic=2";
char const xboref[] PROGMEM = "ref bo";
char const xbapic[] PROGMEM = "ba.pic=6";
char const xbapic2[] PROGMEM = "ba.pic=6";
char const xba2pic[] PROGMEM = "ba.pic=7";
char const xba2pic2[] PROGMEM = "ba.pic=7";
char const xbaref[] PROGMEM = "ref ba";
char const xbnpic[] PROGMEM = "bn.pic=4";
char const xbnpic2[] PROGMEM = "bn.pic=4";
char const xbn2pic[] PROGMEM = "bn.pic=5";
char const xbn2pic2[] PROGMEM = "bn.pic=5";
char const xbnref[] PROGMEM = "ref bn";
char const xl1[] PROGMEM = "l1.txt=\"";
char const xl2[] PROGMEM = "l2.txt=\"";
char const xl3[] PROGMEM = "l3.txt=\"";
char const xl4[] PROGMEM = "l4.txt=\"";
char const xl5[] PROGMEM = "l5.txt=\"";
char const xl6[] PROGMEM = "l6.txt=\"";
char const xl7[] PROGMEM = "l7.txt=\"";
char const xl8[] PROGMEM = "l8.txt=\"";
char const xpth[] PROGMEM = "\"";
char const xwt[] PROGMEM = "wt.txt=\"";
char const xlt[] PROGMEM = "lt.txt=\"";
char const xwf[] PROGMEM = "wf.txt=\"";
char const xlf[] PROGMEM = "lf.txt=\"";
char const init1[] PROGMEM = "bkcmd=0";
char const init2[] PROGMEM = "page 0";
char const init3[] PROGMEM = "baud=";
char const xpage[] PROGMEM = "page ";
char const xt1[] PROGMEM = "t1.txt=\"";
char const xt2[] PROGMEM = "t2.txt=\"";
char const xt3[] PROGMEM = "t3.txt=\"";
char const xt4[] PROGMEM = "t4.txt=\"";
char const xt5[] PROGMEM = "t5.txt=\"";
char const xt6[] PROGMEM = "t6.txt=\"";
char const xt7[] PROGMEM = "t7.txt=\"";
char const xt8[] PROGMEM = "t8.txt=\"";
char const xt9[] PROGMEM = "t9.txt=\"";
char const xt10[] PROGMEM = "t10.txt=\"";
char const xgt1[] PROGMEM = "get t1.txt";
char const xgt2[] PROGMEM = "get t2.txt";
char const xgt3[] PROGMEM = "get t3.txt";
char const xgt4[] PROGMEM = "get t4.txt";
char const xgt5[] PROGMEM = "get t5.txt";
char const xgt6[] PROGMEM = "get t6.txt";
char const xgt7[] PROGMEM = "get t7.txt";
char const xgt8[] PROGMEM = "get t8.txt";
char const xgt9[] PROGMEM = "get t9.txt";
char const xgt10[] PROGMEM = "get t10.txt";
char const xc0[] PROGMEM = "c0.val=";
char const xc1[] PROGMEM = "c1.val=";
char const xgc0[] PROGMEM = "get c0.val";
char const xgc1[] PROGMEM = "get c1.val";
char const xhoursettext[] PROGMEM = "hour.txt=\"";
char const xti[] PROGMEM = "ti.txt=\"";
char const xgti[] PROGMEM = "get ti.txt";
char const xwtred[] PROGMEM = "wt.pco=62225";
char const xwtgreen[] PROGMEM = "wt.pco=38898";
char const xst[] PROGMEM = "st.txt=\"";
char const xn0[] PROGMEM = "n0.val=";
char const xn1[] PROGMEM = "n1.val=";
char const xn2[] PROGMEM = "n2.val=";
char const xn3[] PROGMEM = "n3.val=";
char const xn4[] PROGMEM = "n4.val=";
char const xn5[] PROGMEM = "n5.val=";
char const xn6[] PROGMEM = "n6.val=";
char const xn7[] PROGMEM = "n7.val=";
char const xn8[] PROGMEM = "n8.val=";
char const xn9[] PROGMEM = "n9.val=";
char const xgn0[] PROGMEM = "get n0.val";
char const xgn1[] PROGMEM = "get n1.val";
char const xgn2[] PROGMEM = "get n2.val";
char const xgn3[] PROGMEM = "get n3.val";
char const xgn4[] PROGMEM = "get n4.val";
char const xgn5[] PROGMEM = "get n5.val";
char const xgn6[] PROGMEM = "get n6.val";
char const xgn7[] PROGMEM = "get n7.val";
char const xgn8[] PROGMEM = "get n8.val";
char const xgn9[] PROGMEM = "get n9.val";
char const xc2[] PROGMEM = "c2.val=";
char const xc3[] PROGMEM = "c3.val=";
char const xc4[] PROGMEM = "c4.val=";
char const xc5[] PROGMEM = "c5.val=";
char const xc6[] PROGMEM = "c6.val=";
char const xc7[] PROGMEM = "c7.val=";
char const xc8[] PROGMEM = "c8.val=";
char const xva0[] PROGMEM = "va0.val=";
char const xva1[] PROGMEM = "va1.val=";
char const xva2[] PROGMEM = "va2.val=";
char const xva3[] PROGMEM = "va3.val=";
char const xfill[] PROGMEM = "fill ";
char const xcomma[] PROGMEM = ",";
char const xsp0[] PROGMEM = "vis p0,1";
char const xsp1[] PROGMEM = "vis p1,1";
char const xsp2[] PROGMEM = "vis p2,1";
char const xhp0[] PROGMEM = "vis p0,0";
char const xhp1[] PROGMEM = "vis p1,0";
char const xhp2[] PROGMEM = "vis p2,0";
char const xt0[] PROGMEM = "t0.txt=\"";
char const xltred[] PROGMEM = "lt.pco=62225";
char const xltgreen[] PROGMEM = "lt.pco=38892";
char const xstred[] PROGMEM = "st.pco=62225";
char const xstgreen[] PROGMEM = "st.pco=38898";

PGM_P const nxStrings[] PROGMEM
{
  xhoursettext, //0
  xpth, //1
  xbopic, //2
  xbopic2, //3
  xbo2pic2, //4
  xbo2pic,//5
  xboref, //6
  xbapic,//7
  xbapic2,//8
  xba2pic2,//9
  xba2pic,//10
  xbaref,//11
  xbnpic,//12
  xbnpic2,//13
  xbn2pic2,//14
  xbn2pic,//15
  xbnref, //16
  xl1, //17
  xl2, //18
  xl3,//19
  xl4,//20
  xl5,//21
  xl6,//22
  xl7, //23
  xl8,//24
  xlt,//25
  xwt,//26
  init1, //27
  init2,  //28
  xpage, //29
  xt1, //30
  xt2, //31
  xt3,//32
  xt4,//33
  xt5,  //34
  xt6,  //35
  xt7,  //36
  xt8,  //37
  xt9,  //38
  xt10,  //39
  xgt1, //40
  xgt2, //41
  xgt3,//42
  xgt4,//43
  xgt5,//44
  xgt6,  //45
  xgt7,  //46
  xgt8,  //47
  xgt9,  //48
  xgt10,  //49
  xc0, //50
  xc1, //51
  xgc0, //52
  xgc1, //53
  xti, //54
  xgti, //55
  xwtred, //56
  xwtgreen, //57
  init3, //58
  xst, //59
  xn0, //60
  xn1,
  xn2,
  xn3,
  xn4,
  xn5,
  xn6,
  xn7,
  xn8,
  xn9,
  xgn0, //70
  xgn1,
  xgn2,
  xgn3,
  xgn4,
  xgn5,
  xgn6,
  xgn7,
  xgn8,
  xgn9,
  xc1,//80
  xc2,//81
  xc3,//82
  xc4,//83
  xc5,//84
  xc6,//85
  xva0, //86
  xva1, //87
  xva2,//88
  xva3,//89
  xfill, //90
  xcomma, //91
  xc7, //92
  xc8, //93
  xsp0, //94
  xsp1,//95
  xsp2, //96
  xhp0, //97
  xhp1, //98
  xhp2, //99
  xt0, //100
  xltred, //101
  xltgreen, //102
  xstred, //103
  xstgreen //104


};

char const  xon[] PROGMEM = "A\0";
char const  xoff[] PROGMEM = "S\0";
char const  xnight[] PROGMEM = "P\0";
char const  xsunrise[] PROGMEM = "G\0";
char const  xsunset[] PROGMEM = "H\0";
char const  xdash[] PROGMEM = "-\0";
char const  xslash[]  PROGMEM = "/\0";
char const  xempty[]  PROGMEM = "\0";
char const  xrecover[]  PROGMEM = "D\0";
char const  xfan[] PROGMEM = "U\0";
char const  xup[] PROGMEM = "L\0";
char const  xdown[] PROGMEM = "R\0";
char const  xspace[] PROGMEM = " \0";

char const xcelc[]  = "'C\0";
char const xpercent[]  = "%\0";
char const  xxdash[]  = "-\0";
char const  xxspace[]  = " \0";

PGM_P const nxConstStrings[] PROGMEM
{
  xon, //0
  xoff, //1
  xnight, //2
  xsunrise, //3
  xsunset, //4
  xdash, //5
  xslash, //6
  xempty, //7
  xrecover, //8
  xfan, //9
  xup, //10
  xdown, //11
  xspace //12
};
