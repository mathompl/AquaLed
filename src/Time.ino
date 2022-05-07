/*
   AQUALED date/time functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#include <Arduino.h>

#define _SECS_PER_DAY  (3600UL * 24UL)
#define _DAYS_PER_WEEK (7UL)

#define myDayOfWeek(_time_)  ((( _time_ / _SECS_PER_DAY + 4)  % _DAYS_PER_WEEK)+1) // 1 = Sunday


void getCurrentTime ()
{
        if (currentMillis -  previousRTCCall > RTC_CALL_INTERVAL)
        {
                previousRTCCall = currentMillis;
                currHour = RTC.now().hour ();
                currMinute = RTC.now().minute ();
                currSecond = RTC.now().second ();
                currTime = (long) currHour * 60 * 60 + (long) currMinute * 60 + (long) currSecond;
                if (currTime <=1 ) getMoonPhase ();
        }
}

void rtcSetup ()
{
        Wire.begin();
        Wire.setClock(400000);
        RTC.begin();
        startTimestamp = RTC.now().unixtime ();
        readTimes ();
}

static void readTimes ()
{
        currentMillis = millis();
        getCurrentTime ();
        adjustDST ();
}

static void adjustTime (int h, byte dst)
{
        //  setTime( hour()+h, minute(), second (), day(), month(), year() );
        RTC.adjust(RTC.now() + (long)((long)h * 3600L));
        settings.dst = dst;
        writeEEPROMSettings ();
}

// adjust daylight saving time (european)
static void adjustDST ()
{
        if (currentMillis -  previousSecTimeAdjust > TIME_ADJUST_INTERVAL)
        {
                previousSecTimeAdjust = currentMillis;
                if (myDayOfWeek(RTC.now().unixtime()) == 1 && RTC.now().month() == 3 && RTC.now().day() >= 25 && RTC.now().day() <=31 && RTC.now().hour() == 2 && settings.dst==0)
                        adjustTime (1,1);
                else if (myDayOfWeek(RTC.now().unixtime()) == 1 && RTC.now().month() == 10 && RTC.now().day() >= 25 && RTC.now().day() <=31 && RTC.now().hour() == 3 && settings.dst==1)
                        adjustTime (-1,0);
        }
}

static byte toMoonPhase(int year, int month, int day){
        //Returns 0,1,2 depending on how close is the day to moon phase (1:Phase)
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

static void getMoonPhase ()
{
        moonPhase = toMoonPhase (RTC.now().year(), RTC.now().month(), RTC.now().day());
}
