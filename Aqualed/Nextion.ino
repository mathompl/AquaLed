/*
AQUALED Nextion support functions (c) T. Formanowski 2016-2017
https://github.com/mathompl/AquaLed
*/

#ifndef NO_NEXTION

#include <Arduino.h>
#include "Nextion.h"

// adds integer value to buffer
static void addInt (int value, boolean addComma)
{
        sprintf (buffer + strlen (buffer), "%i", value);
        if (addComma) strcpy_P(buffer + strlen (buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_COMMA])));
}

// fills nextion rectangle
static void fillRect (int x, int y, int w, int h, int color)
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[CMD_FILL])));
        addInt (x, true);
        addInt (y, true);
        addInt (w, true);
        addInt (h, true);
        addInt (color, false);
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
}

static void sendCommandPGMInt (int pgmCommandIndex,  int value, boolean closingParenthesis)
{
        sendCommandPGMInt (pgmCommandIndex,  value, closingParenthesis, false);
}

static void  sendCommandPGMInt (int pgmCommandIndex,  int value, boolean closingParenthesis, boolean leadingZeros)
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[pgmCommandIndex])));
        char tmp[5] = {0};
        memset(tmp, 0, sizeof (tmp));
        if (leadingZeros) sprintf (tmp, "%02d", value); else sprintf (tmp, "%d", value);
        strcpy (buffer + strlen(buffer), tmp);
        if (closingParenthesis) strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
}

static void  sendCommandPGMLong (int pgmCommandIndex,  long value, boolean closingParenthesis)
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[pgmCommandIndex])));
        char tmp[5] = {0};
        memset(tmp, 0, sizeof (tmp));
        sprintf (tmp, "%lu", value);
        strcpy (buffer + strlen(buffer), tmp);
        if (closingParenthesis) strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
}


static void sendCommandPGM_C (int pgmCommandIndex, int pgmConstStringIndex )
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[pgmCommandIndex])));
        strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxConstStrings[pgmConstStringIndex])));
        strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
}

static void sendCommandPGM (int pgmCommandIndex,  char *text, ...)
{
        char* str;
        str = text;
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[pgmCommandIndex])));
        va_list ap;
        va_start(ap, text);
        do {
                strcpy (buffer + strlen (buffer), str);
                str = va_arg(ap, char*);
        } while (str != NULL);
        strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand (buffer);
        va_end(ap);
}

// same  as sendcommandpgm but with user defined buffer size
static void sendCommandPGMbs (int pgmCommandIndex,  byte bufferSize, char *text, ...)
{
        char* str;
        str = text;
        char localBuff[bufferSize];
        memset( localBuff, 0, bufferSize );
        strcpy_P(localBuff, (PGM_P)pgm_read_word(&(nxStrings[pgmCommandIndex])));
        va_list ap;
        va_start(ap, text);
        do {
                strcpy (localBuff + strlen (localBuff), str);
                str = va_arg(ap, char*);
        } while (str != NULL);
        strcpy_P(localBuff + strlen(localBuff), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        strcpy (localBuff + strlen(localBuff), "\0");
        sendCommand(localBuff);
        va_end(ap);
}

// send nextion command without parameters
static void sendCommandPGM (int pgmCommandIndex)
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[pgmCommandIndex])));
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
}

// sned command to NEXTION_BAUD_RATE
static void sendCommand(const char* cmd)
{
        Serial.print(cmd);
        Serial.write(0xFF);
        Serial.write(0xFF);
        Serial.write(0xFF);
        wdt_reset();
}


bool receiveNumber(int *result)
{
        uint8_t temp[8] = {0};
        uint32_t r;
        Serial.setTimeout(500);
        if (sizeof(temp) != Serial.readBytes((char *)temp, sizeof(temp)))
        {
                return false;
        }
        if (temp[0] == NEX_RET_NUMBER_HEAD
            && temp[5] == 0xFF
            && temp[6] == 0xFF
            && temp[7] == 0xFF
            )
        {
                r = ((uint32_t)temp[4] << 24) | ((uint32_t)temp[3] << 16) | ((uint32_t)temp[2] << 8) | ((uint32_t)temp[1]);
                *result = (int) r;
                return true;
        }
        return false;
}

// init nextion lcd
void nexInit(void)
{
        Serial.begin(9600);
        sendCommand("");
        sendCommandPGM(CMD_INIT1);
        sendCommandPGM(CMD_INIT2);
        sendCommandPGMLong(CMD_INIT3, NEXTION_BAUD_RATE, false);
        #ifndef NEXTION_SIMULATOR
        Serial.flush ();
        delay (1000);
        Serial.end ();
        #endif
        Serial.begin(NEXTION_BAUD_RATE);
        toggleButtons ();
        lastTouch = currentTimeSec;
        wdt_reset();
}

void nxReinit()
{
        Serial.begin(9600);
        Serial.flush ();
        Serial.end ();
        sendCommandPGMLong(CMD_INIT3, NEXTION_BAUD_RATE, false);
        Serial.begin(NEXTION_BAUD_RATE);

}

// main touch listener
static void nxTouch()
{
        byte __buffer[7];
        memset(__buffer, 0, sizeof (__buffer));
        byte i = 0;
        byte c;
        byte pid, cid;
        boolean touchEvent = false;
        while (Serial.available() > 0)
        {
                wdt_reset();
                delay(10);
                c = Serial.read();
                wdt_reset();
                if (c == NEX_RET_EVENT_TOUCH_HEAD)
                {

                        touchEvent = true;
                }
                if (touchEvent)
                {
                        __buffer[i] = c;
                        i++;
                }
                if (i > 6)
                {
                        break;
                }
        }
        if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6])
        {
                pid = __buffer[1];
                cid = __buffer[2];
                //  delay(10);
                handlePage (pid, cid);
                return;
        }
        wdt_reset();
}

static void handlePage (byte pid, byte cid)
{
        lastTouch = currentTimeSec;
        switch (pid)
        {
        case PAGE_HOME:
                handleHomePage (cid);
                break;

        case PAGE_CONFIG:
                handleConfigPage (cid);
                break;

        case PAGE_SETTIME:
                handleSetTimePage (cid);
                break;

        case PAGE_SETTINGS:
                handleSettingsPage (cid);
                break;

        case PAGE_PWM:
                handlePWMPage (cid);
                break;

        case PAGE_TEST:
                handleTestPage (cid);
                break;

        case PAGE_SCREENSAVER:
                handleScreenSaver (cid);
                break;

        case PAGE_THERMO:
                handleThermoPage (cid);
                break;

        case PAGE_SCHEDULE:
                handleSchedulePage (cid);
                break;

        case PAGE_PWM_LIST:
                handlePWMListPage (cid);
                break;

        default:
                break;
        }
        lastTouch = currentTimeSec;
}

static void handleScreenSaver (byte cid)
{
        forceRefresh = true;
        sendCommandPGMInt (CMD_SET_PAGE, PAGE_HOME, false);
        nxScreen = PAGE_HOME;
        lastTouch = currentTimeSec;
}

void handleThermoPage (byte cid)
{
        switch (cid)
        {
        // save
        case 9:
                sendCommandPGM (CMD_GET_N0);
                if (!receiveNumber (&t)) return;


                for (int k = 0; k < 8; k++)
                        SETTINGS.ledSensorAddress[k] = sensorsList[t][k];

                sendCommandPGM (CMD_GET_N1);
                if (!receiveNumber (&t)) return;

                for (int k = 0; k < 8; k++)
                        SETTINGS.sumpSensorAddress[k] = sensorsList[t][k];

                sendCommandPGM (CMD_GET_N2);
                if (!receiveNumber (&t)) return;

                for (int k = 0; k < 8; k++)
                        SETTINGS.waterSensorAddress[k] = sensorsList[t][k];

                writeEEPROMSettings ();
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                break;

        //cancel
        case 10:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                break;

        default:
                break;
        }
}

void handleSchedulePage (byte cid)
{
        switch (cid)
        {
        //cancel
        case 1:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                break;

        default:
                break;
        }
}

void handleTestPage (byte cid)
{
        byte p = cid - 3;
        byte c = 0;

        switch (cid)
        {
        // pwms
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 21:
        case 24:
                if (cid == 21)
                {
                        p = 6;
                }
                else if (cid == 24)
                {
                        p = 7;
                }
                else if (cid == 24)
                {
                        p = cid - 3;
                }
                c = 70 + p;
                sendCommandPGM (c);
                if (!receiveNumber (&t)) return;
                pwmChannel[p].pwmTest = mapRound ((byte)t, 0, 100, 0, 255);
                break;


        // close
        case 2:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                testMode = false;
                for (int i = 0; i < PWMS; i++)
                        pwmChannel[i].pwmTest = 0;
                nxScreen = PAGE_CONFIG;
                break;

        default:
                break;
        }
}


void handlePWMPage (byte cid)
{
        byte tmin, tmax, tamb;
        int i;
        switch (cid)
        {
        // save
        case 13:
                sendCommandPGM (CMD_GET_N9);
                if (!receiveNumber (&i)) return;
                if (i < 1 || i > PWMS) return;

                sendCommandPGM (CMD_GET_C0);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmStatus = t;

                sendCommandPGM (CMD_GET_C1);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmKeepLight = t;

                sendCommandPGM (CMD_GET_N0);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmHOn = t;

                sendCommandPGM (CMD_GET_N1);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmMOn = t;

                sendCommandPGM (CMD_GET_N2);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmHOff = t;

                sendCommandPGM (CMD_GET_N3);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmMOff = t;

                sendCommandPGM (CMD_GET_N4);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmSr = t;

                sendCommandPGM (CMD_GET_N5);
                if (!receiveNumber (&t)) return;
                pwmChannel[i - 1].pwmSs = t;

                sendCommandPGM (CMD_GET_N6);
                if (!receiveNumber (&t)) return;

                tmin = mapRound ((byte)t, 0, 100, 0, 255);
                pwmChannel[i - 1].pwmMin = tmin;

                sendCommandPGM (CMD_GET_N7);
                if (!receiveNumber (&t)) return;

                tmax = mapRound ((byte)t, 0, 100, 0, 255);
                pwmChannel[i - 1].pwmMax = tmax;

                sendCommandPGM (CMD_GET_N8);
                if (!receiveNumber (&t)) return;

                tamb = mapRound ((byte)t, 0, 100, 0, 255);
                pwmChannel[i - 1].pwmAmbient = tamb;

                lastTouch = currentTimeSec;
                writeEEPROMPWMConfig (i - 1);
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_PWM_LIST, false);
                nxScreen = PAGE_PWM_LIST;
                break;

        //  cancel
        case 14:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_PWM_LIST, false);
                nxScreen = PAGE_PWM_LIST;
                break;


        default:
                break;
        }
}

static void handleSettingsPage (byte cid)
{
        switch (cid)
        {
        // save
        case 8:
                sendCommandPGM (CMD_GET_N0);
                if (!receiveNumber (&t)) return;
                SETTINGS.max_led_temp = t;

                sendCommandPGM (CMD_GET_N1);
                if (!receiveNumber (&t)) return;
                SETTINGS.max_sump_temp = t;

                sendCommandPGM (CMD_GET_N2);
                if (!receiveNumber (&t)) return;
                SETTINGS.max_water_temp = t;

                sendCommandPGM (CMD_GET_N3);
                if (!receiveNumber (&t)) return;
                SETTINGS.pwmDimmingTime = t;

                sendCommandPGM (CMD_GET_N4);
                if (!receiveNumber (&t)) return;
                SETTINGS.screenSaverTime = t;

                sendCommandPGM (CMD_GET_C0);
                if (!receiveNumber (&t)) return;
                SETTINGS.softDimming  = t;

                writeEEPROMSettings ();
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                break;

        // cancel
        case 9:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                break;

        default:
                break;
        }
}

static void handleSetTimePage (byte cid)
{

        int setHour, setMinute, setYear, setMonth, setDay;
        switch (cid)
        {
        // save
        case 4:
                sendCommandPGM (CMD_GET_N0);
                if (!receiveNumber (&setHour)) return;
                sendCommandPGM (CMD_GET_N1);
                if (!receiveNumber (&setMinute)) return;
                sendCommandPGM (CMD_GET_N2);
                if (!receiveNumber (&setDay)) return;
                sendCommandPGM (CMD_GET_N3);
                if (!receiveNumber (&setMonth)) return;
                sendCommandPGM (CMD_GET_N4);
                if (!receiveNumber (&setYear)) return;
                setTime( setHour, setMinute, 0, setDay, setMonth, setYear );
                RTC.set( now( ) );
                sendCommandPGM (CMD_GET_C0);
                if (!receiveNumber (&t)) return;
                SETTINGS.dst =t;
                readTime ();
                adjustDST ();
                writeEEPROMSettings ();
                lastTouch = currentTimeSec;
                nxScreen = PAGE_CONFIG;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                lastTouch = currentTimeSec;
                break;

        // cancel
        case 5:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                lastTouch = currentTimeSec;
                break;

        default:
                break;
        }
}


static void drawSchedule ()
{

        for (int i = 0; i < PWMS; i++)
        {

                int min_start = (int)pwmChannel[i].pwmHOn * (int)60 + (int)pwmChannel[i].pwmMOn;
                int min_stop = (int)pwmChannel[i].pwmHOff * (int)60 + (int)pwmChannel[i].pwmMOff;

                // background
                if (pwmChannel[i].pwmKeepLight == 1)
                        fillRect (offset * i + startx, starty, width, height, COLOR_DARKBLUE);
                else
                        fillRect (offset * i + startx, starty, width, height, COLOR_RED);

                // off
                if (pwmChannel[i].pwmStatus == 0) continue;

                // light
                boolean midnight = false;
                if (min_stop < min_start) midnight = true;


                if (!midnight)
                {
                        int startL = map (min_start, 0, min_hour, starty, starty + height);
                        int stopL = map (min_stop, 0, min_hour, starty, starty + height ) - startL;
                        fillRect (offset * i + startx, startL, width, stopL, COLOR_GREEN);
                }
                else
                {
                        int startL = map (min_start, 0, min_hour, starty, starty + height);
                        int stopL = map (min_stop, 0, min_hour, starty, starty + height ) - 30;
                        int stopM =  map (min_hour, 0, min_hour, starty, starty + height ) - startL;
                        // start to midnight
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_GREEN);
                        // midnight to end
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_GREEN);
                }
                // sunrise
                int min_sunrise  = min_start + (int)pwmChannel[i].pwmSr;
                if (min_sunrise > min_hour) midnight = true; else midnight = false;

                if (!midnight)
                {
                        int startL = map (min_start, 0, min_hour, starty, starty + height);
                        int stopL = map (min_sunrise, 0, min_hour, starty, starty + height ) - startL;
                        fillRect (offset * i + startx, startL, width, stopL, COLOR_BLUE);
                }
                else
                {
                        int startL = map (min_start, 0, min_hour, starty, starty + height);
                        int stopM =  map (min_hour, 0, min_hour, starty, starty + height ) - startL;
                        int min_sunrise_left = min_sunrise - min_hour;
                        int stopL = map (min_sunrise_left, 0, min_hour, starty, starty + height ) - starty;
                        // start to midnight
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_BLUE);
                        // midnight to end
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_BLUE);
                }
                // sunset
                int min_sunset  = min_stop - (int)pwmChannel[i].pwmSs;
                if (min_sunset < 0) midnight = true; else midnight = false;
                if (!midnight)
                {
                        int startL = map (min_sunset, 0, min_hour, starty, starty + height);
                        int stopL = map (min_stop, 0, min_hour, starty, starty + height ) - startL;
                        fillRect (offset * i + startx, startL, width, stopL, COLOR_BLUE);
                }
                else
                {
                        int min_sunset_left = min_hour + min_sunset;
                        int stopL = map (min_stop, 0, min_hour, starty, starty + height ) - starty;
                        int startL = map (min_sunset_left, 0, min_hour, starty, starty + height);
                        int stopM =  map (min_hour, 0, min_hour, starty, starty + height ) - startL;
                        // zero to end
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_BLUE);
                        // rest till midnight
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_BLUE);
                }
        }
        int min_now = map (tm.Hour * 60 + tm.Minute, 0, min_hour, starty, starty + height);
        fillRect (hour_startx, min_now, hour_stopx, 1, COLOR_YELLOW);
        // siatka dodatkowa
        /*
           for (int i = 0 ; i < 24; i++)
           {
           fillRect (40, 10 * i + starty+1, 180, 1, COLOR_DARKGRAY);

           }*/
}

static void handleConfigPage (byte cid)
{
        byte s;
        byte idxLed, idxSump, idxWater;

        char tempbuff[250] = {0};
        switch (cid)
        {
        // schedule
        case 8:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_SCHEDULE, false);
                nxScreen = PAGE_SCHEDULE;
                drawSchedule ();
                break;
        // close
        case 5:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_HOME, false);
                forceRefresh = true;
                nxScreen = PAGE_HOME;
                toggleButtons();
                break;
        // thermo setup
        case 7:
              #ifndef NO_TEMPERATURE
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_THERMO, false);
                nxScreen = PAGE_THERMO;
                s = discoverOneWireDevices ();
                memset (tempbuff, 0, sizeof (tempbuff));
                sendCommandPGMInt (CMD_SET_VA0, (s - 1), false);
                for (int i = 0; i < s; i++)
                {
                        sprintf(tempbuff + strlen (tempbuff), "%d. ", i);
                        if (i == 0)
                        {
                                strcpy (tempbuff + strlen (tempbuff), "BRAK ");

                        }
                        else
                        {
                                if (sensorsDetected[i])
                                        strcpy (tempbuff + strlen (tempbuff), "* ");
                                for (int k = 0; k < 8; k++)
                                        sprintf (tempbuff + strlen (tempbuff), "%02x ", sensorsList[i][k]);
                        }
                        strcpy  (tempbuff + strlen (tempbuff), "\r\n");
                }

                idxLed = listContains (SETTINGS.ledSensorAddress);
                idxSump = listContains (SETTINGS.sumpSensorAddress);
                idxWater = listContains (SETTINGS.waterSensorAddress);

                if (idxLed == 255) idxLed = 0;
                if (idxSump == 255) idxSump = 0;
                if (idxWater == 255) idxWater = 0;
                sendCommandPGMInt (CMD_SET_N0, idxLed, false);
                sendCommandPGMInt (CMD_SET_N1, idxSump, false);
                sendCommandPGMInt (CMD_SET_N2, idxWater, false);
                sendCommandPGMbs (CMD_SET_T4, 250, tempbuff, NULL);
              #endif
                break;
        // test
        case 6:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_TEST, false);
                nxScreen = PAGE_TEST;
                testMode = true;

                for (int i = 0; i < PWMS; i++)
                {
                        sendCommandPGMInt (60 + i, mapRound (pwmChannel[i].pwmNow, 0, 255, 0, 100), false);
                        sendCommandPGMInt (80 + i, mapRound (pwmChannel[i].pwmNow, 0, 255, 0, 100), false);
                        pwmChannel[i].pwmTest = pwmChannel[i].pwmNow;
                }

                break;

        // hour and date setup
        case 2:

                sendCommandPGMInt (CMD_SET_PAGE, PAGE_SETTIME, false);
                sendCommandPGMInt(CMD_SET_N0, tm.Hour, false);
                sendCommandPGMInt(CMD_SET_N1, tm.Minute, false);
                sendCommandPGMInt(CMD_SET_N2, tm.Day, false);
                sendCommandPGMInt(CMD_SET_N3, tm.Month, false);
                sendCommandPGMInt(CMD_SET_N4, tm.Year + 1970, false);
                sendCommandPGMInt(CMD_SET_C0, SETTINGS.dst, false);
                nxScreen = PAGE_SETTIME;
                break;

        // other settings
        case 4:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_SETTINGS, false);
                sendCommandPGMInt(CMD_SET_N0, SETTINGS.max_led_temp, false);
                sendCommandPGMInt(CMD_SET_N1, SETTINGS.max_sump_temp, false);
                sendCommandPGMInt(CMD_SET_N2, SETTINGS.max_water_temp, false);
                sendCommandPGMInt(CMD_SET_N3, SETTINGS.pwmDimmingTime, false);
                sendCommandPGMInt(CMD_SET_N4, SETTINGS.screenSaverTime, false);
                sendCommandPGMInt(CMD_SET_C0, SETTINGS.softDimming, false);
                nxScreen = PAGE_SETTINGS;
                break;
        // pwm config
        case 1:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_PWM_LIST, false);
                nxScreen = PAGE_PWM;
                break;


        default:
                break;
        }
}

static void handlePWMListPage (byte cid)
{
        byte tmin, tmax, tamb;

        switch (cid)
        {
        // enter pwm settings
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_PWM, false);
                sendCommandPGMInt(CMD_SET_C0, pwmChannel[cid - 1].pwmStatus, false);
                sendCommandPGMInt(CMD_SET_C1, pwmChannel[cid - 1].pwmKeepLight, false);
                sendCommandPGMInt(CMD_SET_N0, pwmChannel[cid - 1].pwmHOn, false);
                sendCommandPGMInt(CMD_SET_N1, pwmChannel[cid - 1].pwmMOn, false);
                sendCommandPGMInt(CMD_SET_N2, pwmChannel[cid - 1].pwmHOff, false);
                sendCommandPGMInt(CMD_SET_N3, pwmChannel[cid - 1].pwmMOff, false);
                sendCommandPGMInt(CMD_SET_N4, pwmChannel[cid - 1].pwmSr, false);
                sendCommandPGMInt(CMD_SET_N5, pwmChannel[cid - 1].pwmSs, false);
                tmin = mapRound (pwmChannel[cid - 1].pwmMin, 0, 255, 0, 100);
                tmax = mapRound (pwmChannel[cid - 1].pwmMax, 0, 255, 0, 100);
                tamb = mapRound (pwmChannel[cid - 1].pwmAmbient, 0, 255, 0, 100);
                sendCommandPGMInt(CMD_SET_N6, tmin, false);
                sendCommandPGMInt(CMD_SET_N7, tmax, false);
                sendCommandPGMInt(CMD_SET_N8, tamb, false);
                sendCommandPGMInt(CMD_SET_N9, cid, false);
                nxScreen = PAGE_PWM;
                break;

        // cancel
        case 10:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                lastTouch = currentTimeSec;
                break;
        default:
                break;
        }
}

static void handleHomePage (byte cid)
{
        switch (cid)
        {

        // config show
        case 24:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                break;

        //toggle night mode
        case 23:
                if (SETTINGS.forceAmbient  == 1 || SETTINGS.forceOFF  == 1) return;
                if (SETTINGS.forceNight == 1)
                {
                        SETTINGS.forceNight = 0;
                        forcePWMRecovery ();
                }
                else
                        SETTINGS.forceNight = 1;

                toggleButtons();
                writeEEPROMSettings ();
                break;

        // ambient toggle
        case 25:
                if (SETTINGS.forceOFF == 1 || SETTINGS.forceNight == 1) return;
                if (SETTINGS.forceAmbient == 1)
                {
                        SETTINGS.forceAmbient = 0;
                        forcePWMRecovery ();
                }
                else SETTINGS.forceAmbient = 1;
                toggleButtons();
                writeEEPROMSettings ();
                break;

        // off/on toggle
        case 22:
                if (SETTINGS.forceOFF == 1)
                {
                        SETTINGS.forceOFF = 0;
                        forcePWMRecovery ();
                }
                else SETTINGS.forceOFF = 1;
                toggleButtons();
                writeEEPROMSettings ();
                break;

        default:
                break;
        }
}
// toggle home page buttons images
static void toggleButtons()
{

        if (SETTINGS.forceOFF == 0)
        {
                sendCommandPGM(CMD_BO_SET_PIC);    sendCommandPGM(CMD_BO_SET_PIC2);    sendCommandPGM(CMD_BO_REFRESH);
        }
        else
        {
                sendCommandPGM(CMD_BO_SET_ALT_PIC2);    sendCommandPGM(CMD_BO_SET_ALT_PIC);    sendCommandPGM(CMD_BO_REFRESH);
        }
        if (SETTINGS.forceAmbient == 0)
        {
                sendCommandPGM(CMD_BA_SET_PIC);    sendCommandPGM(CMD_BA_SET_PIC2);    sendCommandPGM(CMD_BA_REFRESH);
        }
        else
        {
                sendCommandPGM(CMD_BA_SET_ALT_PIC2);    sendCommandPGM(CMD_BA_SET_ALT_PIC);    sendCommandPGM(CMD_BA_REFRESH);
        }

        if (SETTINGS.forceNight == 0)
        {
                sendCommandPGM(CMD_BN_SET_PIC);    sendCommandPGM(CMD_BN_SET_PIC2);    sendCommandPGM(CMD_BN_REFRESH);
        }
        else
        {
                sendCommandPGM(CMD_BN_SET_ALT_PIC2);    sendCommandPGM(CMD_BN_SET_ALT_PIC);    sendCommandPGM(CMD_BN_REFRESH);
        }
}

static void updateWaterTemp() {
    #ifndef NO_TEMPERATURE
        if (nxtemperatureWater != temperatureWater  || forceRefresh)
        {
                if (temperatureWater != TEMP_ERROR)
                {
                        char str_temp[3];
                        dtostrf(temperatureWater, 4, 1, str_temp);
                        sendCommandPGM (CMD_SET_WT, str_temp, xcelc, NULL);
                        nxtemperatureWater = temperatureWater;
                        if (temperatureWater < WATER_TEMPERATURE_MIN || temperatureWater > SETTINGS.max_water_temp)
                                sendCommandPGM(CMD_SET_WT_RED);
                        else
                                sendCommandPGM(CMD_SET_WT_GREEN);
                }
                else
                {
                        sendCommandPGM_C (CMD_SET_WT, STR_DASH);
                }
        }
        #endif
}

static void updateHomePage() {
#ifndef NO_TEMPERATURE
        if (nxtemperatureWater != temperatureWater  || forceRefresh)
        {
                if (temperatureWater != TEMP_ERROR)
                {
                        char str_temp[3];
                        dtostrf(temperatureWater, 4, 1, str_temp);
                        sendCommandPGM (CMD_SET_WT, str_temp, xcelc, NULL);
                        nxtemperatureWater = temperatureWater;
                        if (temperatureWater > SETTINGS.max_water_temp)
                                sendCommandPGM(CMD_SET_WT_RED);
                        else
                                sendCommandPGM(CMD_SET_WT_GREEN);
                }
                else
                {
                        sendCommandPGM_C (CMD_SET_WT, STR_DASH);
                }
        }

        if (nxtemperatureLed != temperatureLed  || forceRefresh)
        {
                if (temperatureLed != TEMP_ERROR)
                {
                        char str_temp[3];
                        dtostrf(temperatureLed, 4, 1, str_temp);
                        sendCommandPGM (CMD_SET_LT, str_temp, xcelc, NULL);
                        nxtemperatureLed = temperatureLed;
                        if (temperatureLed > SETTINGS.max_led_temp)
                                sendCommandPGM(CMD_SET_LT_RED);
                        else
                                sendCommandPGM(CMD_SET_LT_GREEN);
                }
                else
                {
                        sendCommandPGM_C (CMD_SET_LT, STR_DASH);
                }
        }

        if (nxtemperatureSump != temperatureSump  || forceRefresh)
        {
                if (temperatureSump != TEMP_ERROR)
                {
                        char str_temp[3];
                        dtostrf(temperatureSump, 4, 1, str_temp);
                        sendCommandPGM (CMD_SET_ST, str_temp, xcelc, NULL);
                        nxtemperatureSump = temperatureSump;
                        if (temperatureSump > SETTINGS.max_sump_temp)
                                sendCommandPGM(CMD_SET_ST_RED);
                        else
                                sendCommandPGM(CMD_SET_ST_GREEN);
                }
                else
                {
                        sendCommandPGM_C (CMD_SET_ST, STR_DASH);
                }
        }
#endif

        for (int i = 0; i < PWMS; i++)
        {
                if (pwmChannel[i].pwmStatus == 0)
                {
                        sendCommandPGM_C (17 + i, STR_DASH);
                        sendCommandPGM_C (32 + i, STR_SPACE);
                        continue;
                }

                if (pwmNxLast[i] != pwmChannel[i].pwmNow || forceRefresh)
                {
                        char buf2[5] = {0};
                        memset(buf2, 0, sizeof (buf2));
                        byte percent =  mapRound((byte)pwmChannel[i].pwmNow, 0, 255, 0, 100);
                        char buf[3] = {0};
                        itoa(percent, buf, 10);
                        strcpy (buf2 + strlen(buf2), buf);
                        strcpy (buf2 + strlen(buf2), xpercent);
                        sendCommandPGM (17 + i, buf2, NULL);

                        int icon = STR_SPACE;
                        if (pwmChannel[i].isSunrise )
                        {
                                icon = STR_SUNRISE;
                        }
                        else if (pwmChannel[i].isSunset )
                        {
                                icon = STR_SUNSET;
                        }
                        else if (pwmChannel[i].recoverLastState)
                        {
                                icon = STR_RECOVER;
                        }
                        else if (pwmChannel[i].pwmNow < pwmChannel[i].pwmGoal)
                        {
                                icon = STR_UP;
                        }
                        else if (pwmChannel[i].pwmNow > pwmChannel[i].pwmGoal)
                        {
                                icon = STR_DOWN;
                        }
                        else if (pwmChannel[i].pwmNow == 0 || pwmChannel[i].pwmStatus == 0)
                        {
                                icon = STR_OFF;
                        } else if (pwmChannel[i].isNight)
                        {
                                icon = STR_NIGHT;
                        }
                        else if (pwmChannel[i].pwmNow == pwmChannel[i].pwmMax)
                        {
                                icon = STR_ON;
                        }

                        pwmNxLast[i] = pwmChannel[i].pwmNow;
                        sendCommandPGM_C (32+i, icon);

                }
        }
        #ifndef NO_TEMPERATURE
        if (nxwaterFansStatus != waterFansStatus || forceRefresh)
        {
                if (!waterFansStatus)
                        sendCommandPGM_C (CMD_SET_T0, STR_EMPTY);
                else
                        sendCommandPGM_C (CMD_SET_T0, STR_FAN);
                nxwaterFansStatus = waterFansStatus;
        }
        if (nxledFansStatus != ledFansStatus || forceRefresh)
        {
                if (!ledFansStatus)
                        sendCommandPGM_C (CMD_SET_T1, STR_EMPTY);
                else
                        sendCommandPGM_C (CMD_SET_T1, STR_FAN);
                nxledFansStatus = ledFansStatus;
        }

        if (nxsumpFansStatus != sumpFansStatus || forceRefresh)
        {
                if (!sumpFansStatus)
                        sendCommandPGM_C (CMD_SET_T2, STR_EMPTY);
                else
                        sendCommandPGM_C (CMD_SET_T2, STR_FAN);
                nxsumpFansStatus = sumpFansStatus;
        }
        #endif

}

static void timeDisplay(tmElements_t tm) {

        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[CMD_SET_HOUR])));
        if (currentTimeSec % 2 == 0) sprintf(buffer + strlen(buffer), "%02u:%02u", tm.Hour, tm.Minute);
        else sprintf(buffer + strlen(buffer), "%02u %02u", tm.Hour, tm.Minute);
        strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        sendCommand(buffer);
        nxLastHour = tm.Hour;
        nxLastMinute =  tm.Minute;

}
/*
   void debugInfo ()
   {
 #ifndef NO_DEBUG
            char tmp[30] = {0};
            memset(tmp, 0, sizeof (tmp));
            long days=0;
            long hours=0;
            long mins=0;
            long secs=0;
            secs = currentMillis/1000;
            mins=secs/60;
            hours=mins/60;
            days=hours/24;
            secs=secs-(mins*60);
            mins=mins-(hours*60);
            hours=hours-(days*24);
            sprintf (tmp + strlen (tmp), "freemem %i ", freeMemory());
            sprintf (tmp + strlen (tmp), "%02lu %02lu:%02lu:%02lu", days,  hours, mins, secs);
            strcpy (tmp + strlen(tmp), "\0");
            sendCommandPGM (CMD_SET_T1, tmp, NULL, true);
 #endif
   }*/

void nxDisplay ()
{
        if (currentMillis - previousNxReinit > NEXTION_REINIT_TIME)
        {
                previousNxReinit = currentMillis;
                //nxReinit();
        }
        if (currentMillis - previousNxInfo > NX_INFO_RESOLUTION)
        {
                previousNxInfo = currentMillis;

                if (nxScreen == PAGE_SCREENSAVER )
                {
                        timeDisplay(tm);
                        updateWaterTemp();
                        forceRefresh = false;
                }

                if (nxScreen == PAGE_HOME )
                {
                        timeDisplay(tm);
                        updateHomePage();
                        forceRefresh = false;
                }
                // screensaver
                if (currentTimeSec - lastTouch > SETTINGS.screenSaverTime && nxScreen != PAGE_SCREENSAVER && SETTINGS.screenSaverTime > 0
                    && nxScreen != PAGE_TEST
                    && nxScreen != PAGE_SETTINGS
                    && nxScreen != PAGE_PWM
                    && nxScreen != PAGE_SETTIME
                    && nxScreen != PAGE_THERMO
                    && nxScreen != PAGE_SCHEDULE
                    && nxScreen != PAGE_PWM_LIST
                    )
                {
                        sendCommandPGMInt (CMD_SET_PAGE, PAGE_SCREENSAVER, false);
                        nxScreen = PAGE_SCREENSAVER;
                        forceRefresh = true;
                }
        }
        if (lastTouch == 0) lastTouch = currentTimeSec;
}
#endif
