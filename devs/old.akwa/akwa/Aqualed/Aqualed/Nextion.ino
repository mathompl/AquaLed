#include <Arduino.h>

/*
      AquaLed - sterownik oswietlenia akwarium morskiego
       - max 6 PWM,
       - 3 czujnki termeratury,
       - 3 przekazniki na wentylatory
       - wyswietlacz Nextion
     (c) 2016 Tomek Formanowski
     Open Source public domain

     Fragmenty kodu: bluetooth ze sterownika Aqma by Magu, kombatybilnosc zachowana w zakresie obslugi przez bluetooth

 */

// adds integer value to buffer
static void addInt (int x, boolean comma)
{
        sprintf (buffer + strlen (buffer), "%i", x);
        if (comma) strcpy_P(buffer + strlen (buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_COMMA])));

}

// fills nextion rectangle
static void fillRect (int x, int y, int w, int h, int c)
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[CMD_FILL])));
        addInt (x, true);
        addInt (y, true);
        addInt (w, true);
        addInt (h, true);
        addInt (c, false);
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
        wdt_reset();
}

static void sendCommandPGMInt (int i,  int b, boolean pth)
{
        sendCommandPGMInt (i,  b, pth, false);
}

static void  sendCommandPGMInt (int i,  int b, boolean pth, boolean leadingZeros)
{

        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[i])));
        char tmp[5] = {0};
        memset(tmp, 0, sizeof (tmp));
        if (leadingZeros) sprintf (tmp, "%02d", b);
        else
                sprintf (tmp, "%d", b);
        strcpy (buffer + strlen(buffer), tmp);
        if (pth) strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        strcpy (buffer + strlen(buffer), "\0");
        //strcat (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
        wdt_reset();
}

static void sendCommandPGM_C (int i, int x )
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[i])));
        strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxConstStrings[x])));
        strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        strcpy (buffer + strlen(buffer), "\0");
        sendCommand(buffer);
        wdt_reset();
}

static void sendCommandPGM (int i,  char *b, ...)
{
        char* str;
        str = b;
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[i])));
        va_list ap;
        va_start(ap, b);
        do {
                strcpy (buffer + strlen (buffer), str);
                str = va_arg(ap, char*);
        } while (str != NULL);
        strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        //strcat (buffer , "\0");
        //strcat (buffer , NULL);
        sendCommand (buffer);
        wdt_reset();
        va_end(ap);
}

static void sendCommandPGMbs (int i,  byte bs, char *b, ...)
{
        char* str;
        str = b;
        char localBuff[bs];
        memset( localBuff, 0, bs );

        strcpy_P(localBuff, (PGM_P)pgm_read_word(&(nxStrings[i])));
        va_list ap;
        va_start(ap, b);
        do {
                strcpy (localBuff + strlen (localBuff), str);
                str = va_arg(ap, char*);
        } while (str != NULL);
        strcpy_P(localBuff + strlen(localBuff), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
        //strcat (localBuff , "\0");
//  strcat (localBuff , NULL);
        sendCommand(localBuff);
        wdt_reset();
        va_end(ap);
}

static void sendCommandPGM (int i)
{
        memset(buffer, 0, sizeof (buffer));
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[i])));
        sendCommand(buffer);
        wdt_reset();
}

void nexInit(void)
{
        Serial.begin(9600);
        sendCommand("");
        sendCommandPGM(CMD_INIT1);
        sendCommandPGM(CMD_INIT2);
        sendCommandPGM(CMD_INIT3);
        Serial.begin(115200);
        toggleButtons ();
        lastTouch = currentTimeSec;
        wdt_reset();
}

static void sendCommand(const char* cmd)
{
        Serial.print(cmd);
        Serial.write(0xFF);
        Serial.write(0xFF);
        Serial.write(0xFF);
}

// warning - global result and value!
int recvRetNumber()
{

        uint8_t temp[8] = {0};
        uint32_t r;
        Serial.setTimeout(500);
        if (sizeof(temp) != Serial.readBytes((char *)temp, sizeof(temp)))
        {
                recvRes = false;
                return -1;
        }

        if (temp[0] == NEX_RET_NUMBER_HEAD
            && temp[5] == 0xFF
            && temp[6] == 0xFF
            && temp[7] == 0xFF
            )
        {
                r = ((uint32_t)temp[4] << 24) | ((uint32_t)temp[3] << 16) | ((uint32_t)temp[2] << 8) | ((uint32_t)temp[1]);
                recvRes = true;
                return (int)r;

        }
        recvRes = false;
        return -1;
}

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
                delay(10);
                handlePage (pid, cid);
                return;
        }
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
        // zapis
        case 9:
                sendCommandPGM (CMD_GET_N0);
                t = recvRetNumber ();
                if (!recvRes) return;

                for (int k = 0; k < 8; k++)
                        SETTINGS.ledSensorAddress[k] = sensorsList[t][k];

                sendCommandPGM (CMD_GET_N1);
                t = recvRetNumber ();
                if (!recvRes) return;
                for (int k = 0; k < 8; k++)
                        SETTINGS.sumpSensorAddress[k] = sensorsList[t][k];

                sendCommandPGM (CMD_GET_N2);
                t = recvRetNumber ();
                if (!recvRes) return;
                for (int k = 0; k < 8; k++)
                        SETTINGS.waterSensorAddress[k] = sensorsList[t][k];

                writeEEPROMSettings ();
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;
                break;

        //anulowane
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
        //anulowane
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
                t = recvRetNumber ();
                pwm_list[p].pwmTest = mapRound ((byte)t, 0, 100, 0, 255);

                break;


        // powrót
        case 2:
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                testMode = false;
                for (int i = 0; i < PWMS; i++)
                        pwm_list[i].pwmTest = 0;
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
        // zapis ustawien
        case 13:
                sendCommandPGM (CMD_GET_N9);
                i = recvRetNumber ();
                if (!recvRes) return;

                if (i < 1 || i > PWMS) return;

                sendCommandPGM (CMD_GET_C0);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmStatus = t;

                sendCommandPGM (CMD_GET_C1);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmKeepLight = t;

                sendCommandPGM (CMD_GET_N0);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmHOn = t;

                sendCommandPGM (CMD_GET_N1);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmMOn = t;

                sendCommandPGM (CMD_GET_N2);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmHOff = t;

                sendCommandPGM (CMD_GET_N3);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmMOff = t;

                sendCommandPGM (CMD_GET_N4);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmSr = t;


                sendCommandPGM (CMD_GET_N5);
                t = recvRetNumber ();
                if (!recvRes) return;
                pwm_list[i - 1].pwmSs = t;

                sendCommandPGM (CMD_GET_N6);
                t = recvRetNumber ();
                if (!recvRes) return;
                tmin = mapRound ((byte)t, 0, 100, 0, 255);
                pwm_list[i - 1].pwmMin = tmin;

                sendCommandPGM (CMD_GET_N7);
                t = recvRetNumber ();
                if (!recvRes) return;
                tmax = mapRound ((byte)t, 0, 100, 0, 255);
                pwm_list[i - 1].pwmMax = tmax;

                sendCommandPGM (CMD_GET_N8);
                t = recvRetNumber ();
                if (!recvRes) return;
                tamb = mapRound ((byte)t, 0, 100, 0, 255);
                pwm_list[i - 1].pwmAmbient = tamb;

                lastTouch = currentTimeSec;
                writeEEPROMPWMConfig (i - 1);
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_PWM_LIST, false);
                nxScreen = PAGE_PWM_LIST;

                break;

        // wyjdz
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
        // zapis ustawien
        case 8:
                sendCommandPGM (CMD_GET_N0);
                t = recvRetNumber ();
                if (!recvRes) return;
                SETTINGS.max_led_temp = t;

                sendCommandPGM (CMD_GET_N1);
                t = recvRetNumber ();
                if (!recvRes) return;
                SETTINGS.max_sump_temp = t;

                sendCommandPGM (CMD_GET_N2);
                t = recvRetNumber ();
                if (!recvRes) return;
                SETTINGS.max_water_temp = t;

                sendCommandPGM (CMD_GET_N3);
                t = recvRetNumber ();
                if (!recvRes) return;
                SETTINGS.pwmDimmingTime = t;

                sendCommandPGM (CMD_GET_N4);
                t = recvRetNumber ();
                if (!recvRes) return;
                SETTINGS.screenSaverTime = t;

                sendCommandPGM (CMD_GET_C0);
                t = recvRetNumber ();
                if (!recvRes) return;
                SETTINGS.softDimming  = t;
                writeEEPROMSettings ();
                lastTouch = currentTimeSec;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                nxScreen = PAGE_CONFIG;

                break;

        // wyjdz
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

        static int setHour, setMinute, setSecond, setYear, setMonth, setDay;
        switch (cid)
        {
        // zapis daty
        case 4:
                sendCommandPGM (CMD_GET_N0);
                setHour = recvRetNumber ();
                if (!recvRes) return; ;

                sendCommandPGM (CMD_GET_N1);
                setMinute = recvRetNumber ();
                if (!recvRes) return;

                setSecond = 0;

                sendCommandPGM (CMD_GET_N2);
                setDay = recvRetNumber ();
                if (!recvRes) return;

                sendCommandPGM (CMD_GET_N3);
                setMonth = recvRetNumber ();
                if (!recvRes) return;

                sendCommandPGM (CMD_GET_N4);
                setYear = recvRetNumber ();
                if (!recvRes) return;

                setTime( setHour, setMinute, setSecond, setDay, setMonth, setYear );
                RTC.set( now( ) );

                sendCommandPGM (CMD_GET_C0);
                if (!recvRes) return;
                SETTINGS.dst = recvRetNumber ();
                readTime ();
                adjustDST ();
                writeEEPROMSettings ();
                lastTouch = currentTimeSec;
                nxScreen = PAGE_CONFIG;
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_CONFIG, false);
                lastTouch = currentTimeSec;
                break;

        // wyjdz
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

                int min_start = (int)pwm_list[i].pwmHOn * (int)60 + (int)pwm_list[i].pwmMOn;
                int min_stop = (int)pwm_list[i].pwmHOff * (int)60 + (int)pwm_list[i].pwmMOff;

                // pasek tlo
                if (pwm_list[i].pwmKeepLight == 1)
                        fillRect (offset * i + startx, starty, width, height, COLOR_DARKBLUE);
                else
                        fillRect (offset * i + startx, starty, width, height, COLOR_RED);

                // off
                if (pwm_list[i].pwmStatus == 0) continue;

                // praca
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
                        // od startu do polnocy
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_GREEN);
                        // od północy do stopu
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_GREEN);
                }
                // wschod
                int min_sunrise  = min_start + (int)pwm_list[i].pwmSr;
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
                        // od startu do polnocy
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_BLUE);
                        // od pólnocy do końca
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_BLUE);
                }
                // zachod
                int min_sunset  = min_stop - (int)pwm_list[i].pwmSs;
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
                        // od zera do konca
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_BLUE);
                        // pozostalosc do polnocy
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_BLUE);
                }
        }
        int min_now = map (tm.Hour * 60 + tm.Minute, 0, min_hour, starty, starty + height);
        fillRect (40, min_now, 180, 1, COLOR_YELLOW);
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
        // harmonogram
        case 8:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_SCHEDULE, false);
                nxScreen = PAGE_SCHEDULE;
                // rysowanie harmonogramu
                drawSchedule ();
                break;
        // powrot
        case 5:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_HOME, false);
                forceRefresh = true;
                nxScreen = PAGE_HOME;
                toggleButtons();
                break;
        // konfiguracja termometrow
        case 7:
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
                break;
        // test
        case 6:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_TEST, false);
                nxScreen = PAGE_TEST;
                testMode = true;

                for (int i = 0; i < PWMS; i++)
                {
                        sendCommandPGMInt (60 + i, mapRound (pwm_list[i].pwmNow, 0, 255, 0, 100), false);
                        sendCommandPGMInt (80 + i, mapRound (pwm_list[i].pwmNow, 0, 255, 0, 100), false);
                        pwm_list[i].pwmTest = pwm_list[i].pwmNow;
                }

                break;

        // ustawienie godziny
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

        // inne ustawienia
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
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
                sendCommandPGMInt (CMD_SET_PAGE, PAGE_PWM, false);
                sendCommandPGMInt(CMD_SET_C0, pwm_list[cid - 1].pwmStatus, false);
                sendCommandPGMInt(CMD_SET_C1, pwm_list[cid - 1].pwmKeepLight, false);
                sendCommandPGMInt(CMD_SET_N0, pwm_list[cid - 1].pwmHOn, false);
                sendCommandPGMInt(CMD_SET_N1, pwm_list[cid - 1].pwmMOn, false);
                sendCommandPGMInt(CMD_SET_N2, pwm_list[cid - 1].pwmHOff, false);
                sendCommandPGMInt(CMD_SET_N3, pwm_list[cid - 1].pwmMOff, false);
                sendCommandPGMInt(CMD_SET_N4, pwm_list[cid - 1].pwmSr, false);
                sendCommandPGMInt(CMD_SET_N5, pwm_list[cid - 1].pwmSs, false);
                tmin = mapRound (pwm_list[cid - 1].pwmMin, 0, 255, 0, 100);
                tmax = mapRound (pwm_list[cid - 1].pwmMax, 0, 255, 0, 100);
                tamb = mapRound (pwm_list[cid - 1].pwmAmbient, 0, 255, 0, 100);
                sendCommandPGMInt(CMD_SET_N6, tmin, false);
                sendCommandPGMInt(CMD_SET_N7, tmax, false);
                sendCommandPGMInt(CMD_SET_N8, tamb, false);
                sendCommandPGMInt(CMD_SET_N9, cid, false);
                nxScreen = PAGE_PWM;
                break;

        // wyjdz
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
                if (SETTINGS.forceNight == 1) SETTINGS.forceNight = 0; else SETTINGS.forceNight = 1;
                toggleButtons();
                writeEEPROMSettings ();
                break;

        // ambient toggle
        case 25:
                if (SETTINGS.forceOFF == 1 || SETTINGS.forceNight == 1) return;
                if (SETTINGS.forceAmbient == 1) SETTINGS.forceAmbient = 0; else SETTINGS.forceAmbient = 1;
                toggleButtons();
                writeEEPROMSettings ();
                break;

        // off/on toggle
        case 22:
                if (SETTINGS.forceOFF == 1) SETTINGS.forceOFF = 0; else SETTINGS.forceOFF = 1;
                toggleButtons();
                writeEEPROMSettings ();
                break;

        default:
                break;
        }
}
// obrazki
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

        if (nxtemperatureAqua != temperatureAqua  || forceRefresh)
        {
                if (temperatureAqua != TEMP_ERROR)
                {
                        char str_temp[3];
                        dtostrf(temperatureAqua, 4, 1, str_temp);
                        sendCommandPGM (CMD_SET_WT, str_temp, xcelc, NULL);
                        nxtemperatureAqua = temperatureAqua;
                        if (temperatureAqua < 25 || temperatureAqua > SETTINGS.max_water_temp)
                                sendCommandPGM(CMD_SET_WT_RED);
                        else
                                sendCommandPGM(CMD_SET_WT_GREEN);

                }
                else
                {
                        sendCommandPGM_C (CMD_SET_WT, STR_DASH);
                }
        }
}

static void updateInfo() {
        if (nxtemperatureAqua != temperatureAqua  || forceRefresh)
        {
                if (temperatureAqua != TEMP_ERROR)
                {
                        char str_temp[3];
                        dtostrf(temperatureAqua, 4, 1, str_temp);
                        sendCommandPGM (CMD_SET_WT, str_temp, xcelc, NULL);
                        nxtemperatureAqua = temperatureAqua;
                }
                else
                {
                        sendCommandPGM_C (CMD_SET_WT, STR_DASH);
                }
        }

        if (nxtemperatureFans != temperatureFans  || forceRefresh)
        {
                if (temperatureFans != TEMP_ERROR)
                {
                        char str_temp[3];
                        dtostrf(temperatureFans, 4, 1, str_temp);
                        sendCommandPGM (CMD_SET_LT, str_temp, xcelc, NULL);
                        nxtemperatureFans = temperatureFans;
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
                }
                else
                {
                        sendCommandPGM_C (CMD_SET_ST, STR_DASH);
                }
        }

        for (int i = 0; i < PWMS; i++)
        {
                if (pwm_list[i].pwmLast != pwm_list[i].pwmNow || forceRefresh)
                {

                        if (pwm_list[i].pwmNow == 0 || pwm_list[i].pwmStatus == 0)
                        {
                                sendCommandPGM_C(17 + i, STR_OFF);
                        }
                        else if (pwm_list[i].pwmNow == pwm_list[i].pwmMin && pwm_list[i].pwmKeepLight )
                        {
                                sendCommandPGM_C (17 + i, STR_NIGHT);
                        }
                        else if (pwm_list[i].isSunrise )
                        {
                                sendCommandPGM_C (17 + i, STR_SUNRISE);
                        }
                        else if (pwm_list[i].isSunset )
                        {
                                sendCommandPGM_C (17 + i, STR_SUNSET);
                        }
                        else
                        {
                                byte percent =  mapRound((int)pwm_list[i].pwmNow, 0, 255, 0, 100);
                                char buf[3] = {0};
                                itoa(percent, buf, 10);
                                sendCommandPGM (17 + i, buf, xpercent, NULL);
                        }
                        pwm_list[i].pwmLast = pwm_list[i].pwmNow;
                }
        }
        if (nxwaterFansStatus != waterFansStatus || forceRefresh)
        {
                if (waterFansStatus)
                        sendCommandPGM (CMD_SHOW_P0);
                else
                        sendCommandPGM (CMD_HIDE_P0);
                nxwaterFansStatus = waterFansStatus;
        }
        if (nxledFansStatus != ledFansStatus || forceRefresh)
        {
                if (ledFansStatus)
                        sendCommandPGM (CMD_SHOW_P1);
                else
                        sendCommandPGM (CMD_HIDE_P1);
                nxledFansStatus = ledFansStatus;
        }

        if (nxsumpFansStatus != sumpFansStatus || forceRefresh)
        {
                if (sumpFansStatus)
                        sendCommandPGM (CMD_SHOW_P2);
                else
                        sendCommandPGM (CMD_HIDE_P2);
                nxsumpFansStatus = sumpFansStatus;
        }

}



static void timeDisplay(tmElements_t tm) {
        // if (nxLastMinute != tm.Minute || nxLastHour != tm.Hour || forceRefresh)
        {
                memset(buffer, 0, sizeof (buffer));
                strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[CMD_SET_HOUR])));
                if (currentTimeSec % 2 == 0) sprintf(buffer + strlen(buffer), "%02u:%02u", tm.Hour, tm.Minute);
                else sprintf(buffer + strlen(buffer), "%02u %02u", tm.Hour, tm.Minute);
                strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[CMD_PARENTH])));
                sendCommand(buffer);
                nxLastHour = tm.Hour;
                nxLastMinute =  tm.Minute;
        }
}

void nxDisplay ()
{
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
                        updateInfo();
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
