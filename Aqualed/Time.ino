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

                if (dayOfWeek(now()) == 1 && month() == 3 && day() >= 25 && day() <=31 && hour() == 1 && SETTINGS.dst==0)
                {
                        //time_t t = makeTime(tm);
                        //t += 3600;
                        //setTime (t);
                        setTime( hour()+1, minute(), second (), day(), month(), year() );
                        RTC.set( now () );
                        SETTINGS.dst = 1;
                        writeEEPROMSettings ();
                }
                else if (dayOfWeek(now()) == 1 && month() == 10 && day() >= 25 && day() <=31 && hour() == 1 && SETTINGS.dst==1)
                {
                        //time_t t = makeTime(tm);
                        //t -= 3600;
                        //setTime (t);
                        setTime( hour()-1, minute(), second (), day(), month(), year() );
                        RTC.set( now () );
                        SETTINGS.dst = 0;
                        writeEEPROMSettings ();
                }
        }
}
