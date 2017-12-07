/*
   AQUALED date/time functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */

#include <Arduino.h>
#define dayOfWeek(_time_)  ((( _time_ / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)+1) // 1 = Sunday

void readTime ()
{
        currentMillis = millis();
        if (RTC.read(tm))
                adjustDST ();
}

// adjust daylight saving time (european)
void adjustDST ()
{
        if (currentMillis -  previousSecTimeAdjust > TIME_ADJUST_INTERVAL)
        {
                previousSecTimeAdjust = currentMillis;
                getMoonPhase ();
                if (dayOfWeek(now()) == 1 && month() == 3 && day() >= 25 && day() <=31 && hour() == 1 && settings.dst==0)
                {
                        setTime( hour()+1, minute(), second (), day(), month(), year() );
                        RTC.set( now () );
                        settings.dst = 1;
                        writeEEPROMSettings ();
                }
                else if (dayOfWeek(now()) == 1 && month() == 10 && day() >= 25 && day() <=31 && hour() == 1 && settings.dst==1)
                {
                        setTime( hour()-1, minute(), second (), day(), month(), year() );
                        RTC.set( now () );
                        settings.dst = 0;
                        writeEEPROMSettings ();
                }
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
        moonPhase = toMoonPhase (year(), month(), day());
}
