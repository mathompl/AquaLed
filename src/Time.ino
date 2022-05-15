/*
   AQUALED date/time functions (c) T. Formanowski 2016-2022
   https://github.com/mathompl/AquaLed
 */
#include <time.h>

_Time::_Time (DataStorage *_dataStorage)
{
        __dataStorage = _dataStorage;
}


void _Time::begin ()
{
        Wire.begin();
        Wire.setClock(400000);
        rtcAvailable =rtc.begin();
        startTimestamp = rtc.now().unixtime ();
        read ();
        getMoonPhase ();      
        benchmarkStartMillis = millis ();
}

// reads RTC
void _Time::readCurrentTime ()
{
        if (rtcAvailable)
        {
                if (currentMillis -  previousRTCCall > RTC_CALL_INTERVAL || previousRTCCall == 0)
                {
                        previousRTCCall = currentMillis;
                        currHour = rtc.now().hour ();
                        currMinute = rtc.now().minute ();
                        currSecond = rtc.now().second ();
                        currYear = rtc.now().year();
                        currMonth = rtc.now().month();
                        currDay = rtc.now().day();
                        unixTime = rtc.now().unixtime();
                        currTime = (long) currHour * 60 * 60 + (long) currMinute * 60 + (long) currSecond;
                        if (currTime <=1 ) getMoonPhase ();
                }
        }
}

void _Time::read ()
{
        currentMillis = millis();
        readCurrentTime();
        adjustDST ();
        //    doBenchmark ();
}

void _Time::doBenchmark ()
{
        if (benchmarkIteration >= BENCHMARK_ITERATIONS)
        {
                benchmarkResult = ( (currentMillis - benchmarkStartMillis) *1000 ) / benchmarkIteration;
                benchmarkIteration = 0;
                benchmarkStartMillis = currentMillis;
        }
        benchmarkIteration++;
}

void _Time::adjustTime (int h, byte dst)
{
        rtc.adjust(rtc.now() + (long)((long)h * 3600L));
        settings.dst = dst;
        __dataStorage->writeEEPROMSettings ();
}

// adjust daylight saving time (european)
void _Time::adjustDST ()
{
        if (currentMillis -  previousSecTimeAdjust > TIME_ADJUST_INTERVAL)
        {
                previousSecTimeAdjust = currentMillis;
                if (myDayOfWeek(rtc.now().unixtime()) == 1 && rtc.now().month() == 3 && rtc.now().day() >= 25 && rtc.now().day() <=31 && rtc.now().hour() == 2 && settings.dst==0)
                        adjustTime (1,1);
                else if (myDayOfWeek(rtc.now().unixtime()) == 1 && rtc.now().month() == 10 && rtc.now().day() >= 25 && rtc.now().day() <=31 && rtc.now().hour() == 3 && settings.dst==1)
                        adjustTime (-1,0);
        }
}

//Returns 0,1,2 depending on how close is the day to moon phase (1:Phase)
byte _Time::toMoonPhase(int year, int month, int day){
        int r = year % 100;
        r %= 19;
        if (r>9) { r -= 19;}
        r = ((r * 11) % 30) + month + day;
        if (month<3) {r += 2;}
        r = int(floor(r-((year<2000) ? 4 : 8.3) +0.5))%30;
        int res;
        (r < 0) ? res=r+30 : res=r; //0-29
        return res;
}

void _Time::getMoonPhase ()
{
        moonPhase = toMoonPhase (rtc.now().year(), rtc.now().month(), rtc.now().day());
}

void _Time::adjust (DateTime time)
{
        rtc.adjust(time);
}

uint8_t _Time::getHour ()
{
        return currHour;
}
uint8_t _Time::getMinute ()
{
        return currMinute;
}
uint8_t _Time::getSecond ()
{
        return currSecond;
}
long _Time::getCurrentTime ()
{
        return currTime;
}

uint8_t _Time::getDay ()
{
        return currDay;
}
uint8_t _Time::getMonth ()
{
        return currMonth;
}
int _Time::getYear ()
{
        return currYear;
};

long _Time::getUnixTime ()
{
        return unixTime;
}
boolean _Time::isRTC()
{
        return rtcAvailable;
}

byte _Time::getMoonPhaseValue ()
{
        return moonPhases[moonPhase];
}

// micro seconds
byte _Time::getBenchmark ()
{
        return benchmarkResult;
}
