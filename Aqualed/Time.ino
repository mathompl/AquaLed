/*
AQUALED date/time functions (c) T. Formanowski 2016-2017
https://github.com/mathompl/AquaLed
*/

#include <Arduino.h>

bool isSummer() {        
        return ( (month () == 3  && (hour () + 24 * day()) >= (1 + 24 * (31 - (5 * (year() + 1970) / 4 + 4) % 7))) ||
                 (month () == 10 && (hour () + 24 * day()) < (1 + 24 * (31 - (5 * (year () + 1970) / 4 + 1) % 7))));
}

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
                if (isSummer() && !SETTINGS.dst)
                {
                        time_t t = makeTime(tm);
                        t += 3600;
                        setTime (t);
                        RTC.set( now () );
                        SETTINGS.dst = 1;
                        writeEEPROMSettings ();
                }
                else if (!isSummer() && SETTINGS.dst)
                {
                        time_t t = makeTime(tm);
                        t -= 3600;
                        setTime (t);
                        RTC.set( now () );
                        SETTINGS.dst = 0;
                        writeEEPROMSettings ();
                }
        }
}
