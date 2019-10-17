#include <Arduino.h>

bool isSummer() {
        if (tm.Month < 3 || tm.Month > 10) return false;
        if (tm.Month > 3 && tm.Month < 10) return true;
        return ( (tm.Month == 3  && (tm.Hour + 24 * tm.Day) >= (1 + 24 * (31 - (5 * (tm.Year + 1970) / 4 + 4) % 7))) ||
                 (tm.Month == 10 && (tm.Hour + 24 *  tm.Day) < (1 + 24 * (31 - (5 * (tm.Year + 1970) / 4 + 1) % 7))));
}

void readTime ()
{
        currentMillis = millis();
        if (RTC.read(tm)) {
                currentTimeSec = (long(tm.Hour) * 3600) + (long(tm.Minute) * 60) + long(tm.Second);
                adjustDST ();
        }
}

void adjustDST ()
{
        if (currentTimeSec -  previousSecTimeAdjust > TIME_ADJUST_INTERVAL)
        {
                previousSecTimeAdjust = currentTimeSec;
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
