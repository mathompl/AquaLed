#ifndef TIME_H
#define TIME_H
#include "RTClib.h"
#include <DS18B20.h>
#include "datastorage.h"


#define _SECS_PER_DAY  (3600UL * 24UL)
#define _DAYS_PER_WEEK (7UL)
#define myDayOfWeek(_time_)  ((( _time_ / _SECS_PER_DAY + 4)  % _DAYS_PER_WEEK)+1) // 1 = Sunday

#define BENCHMARK_ITERATIONS 100000

// time variables
long unsigned currentMillis = 0;
unsigned long previousRTCCall = 0;
unsigned long previousPwmResolution = 0;
unsigned long previousNxInfo = 0;
unsigned long previousTemperatureRead = 0;
unsigned long previousTemperatureRequest = 0;
unsigned long previousFansControl = 0;
unsigned long previousMillisFans = 0;
unsigned long previousMillisNextion = 0;
unsigned long previousSecTimeAdjust = 0;
unsigned long lastTouch = 0;
uint32_t startTimestamp = 0;

const byte moonPhases[] = {0,  1,  4,  9,  16,  25, 36, 50, 58, 69,
                           79, 88, 94, 99, 100, 99, 95, 90, 83, 75,
                           66, 57, 50, 38, 29,  21, 13, 7,  3,  1};

class _Time
{
public:
      _Time (DataStorage *_dataStorage);
      void begin ();
      void read ();
      void adjustTime (int h, byte dst);
      void adjustDST ();
      void adjust (DateTime time);
      uint8_t getHour ();
      uint8_t getMinute ();
      uint8_t getSecond ();
      uint8_t getDay ();
      uint8_t getMonth ();
      int getYear ();
      long getUnixTime ();
      long getCurrentTime ();
      void readCurrentTime();
      boolean isRTC();
      byte getMoonPhaseValue ();
      byte getBenchmark ();

private:
// time variables
      DataStorage *__dataStorage;
      byte moonPhase = 0;
      byte benchmarkResult = 0;
      long benchmarkIteration = 0;
      long benchmarkStartMillis = 0;
      byte toMoonPhase(int year, int month, int day);
      boolean rtcAvailable = false;
      RTC_DS3231 rtc;
      long currTime = 0;
      uint8_t currHour = 0;
      uint8_t currMinute = 0;
      uint8_t currSecond = 0;
      uint8_t currDay = 0;
      uint8_t currMonth = 0;
      int currYear = 0;
      long unixTime = 0;
      void getMoonPhase ();
      void doBenchmark ();
};
#endif
