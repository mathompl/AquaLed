#define NEX_RET_CMD_FINISHED            (0x01)
#define NEX_RET_EVENT_TOUCH_HEAD            (0x65)   
#define NEX_EVENT_POP   (0x00)  
#define NEX_RET_STRING_HEAD                 (0x70)
#define NEX_RET_NUMBER_HEAD                 (0x71)

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
char const xpth[] PROGMEM = "\"";

char const xwt[] PROGMEM = "wt.txt=\"";
char const xlt[] PROGMEM = "lt.txt=\"";
char const xwf[] PROGMEM = "wf.txt=\"";
char const xlf[] PROGMEM = "lf.txt=\"";
char const init1[] PROGMEM = "bkcmd=1";
char const init2[] PROGMEM = "page 0";

char const xpage[] PROGMEM = "page ";

char const xt1[] PROGMEM = "t1.txt=\"";
char const xt2[] PROGMEM = "t2.txt=\"";
char const xt3[] PROGMEM = "t6.txt=\"";
char const xt4[] PROGMEM = "t6.txt=\"";
char const xt5[] PROGMEM = "t7.txt=\"";

char const xgt1[] PROGMEM = "get t1.txt";
char const xgt2[] PROGMEM = "get t2.txt";
char const xgt3[] PROGMEM = "get t5.txt";
char const xgt4[] PROGMEM = "get t6.txt";
char const xgt5[] PROGMEM = "get t7.txt";



char const xhoursettext[] PROGMEM = "hour.txt=\"";

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
  xwt, //23
  xwf,//24
  xlt,//25
  xlf,//26
  init1, //27
  init2,  //28
  xpage, //29
  xt1, //30
  xt2, //31
  xt3,//32
  xt4,//33
  xt5,  //34
  xgt1, //35
  xgt2, //36
  xgt3,//37
  xgt4,//38
  xgt5  //39

};


char xon[] = "ON";
char xoff[] = "OFF";
char xnight[] = "NIGHT";
char xpercent[] = "%";
char xdash[] = "-";
char xcelc[] = "'C";
