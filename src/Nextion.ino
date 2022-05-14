/*
   AQUALED Nextion support functions (c) T. Formanowski 2016-2017
   https://github.com/mathompl/AquaLed
 */



#include <Arduino.h>
#include "aqualed.h"
#include "Nextion.h"

#ifndef NO_NEXTION
// init nextion lcd

Nextion::Nextion (_Time *_time, PWM *_pwm, Sensors* _sensors, DataStorage *_dataStorage)
{
        __time = _time;
        __pwm = _pwm;
        __sensors = _sensors;
        __dataStorage = _dataStorage;
}

void Nextion::begin(void)
{
        NEXTION_BEGIN (9600);
        sendNextionEOL ();
        setPage(PAGE_HOME);
        setInt (NX_FIELD_BAUDS, NX_FIELD_EMPTY, (long)NEXTION_BAUD_RATE);
        NEXTION_FLUSH ();
        delay (50);
        NEXTION_BEGIN ((long)NEXTION_BAUD_RATE);
        setInt (NX_FIELD_BKCMD, NX_FIELD_EMPTY, (long)0);
        setPage (PAGE_HOME);
        refreshPWMNames ();
        toggleButtons ();
        forceRefresh = true;
        NEXTION_FLUSH ();
        __last_response = currentMillis;
        __last_keepAlive = currentMillis;
        lastTouch = currentMillis;
}

void Nextion::reconnect ()
{
        NEXTION_FLUSH ();
        NEXTION_END();
        begin ();
}

void Nextion::refreshPWMNames ()
{
        // init names
        for (byte i = 0; i < PWMS; i++)
        {
                setText (PAGE_HOME, NX_FIELD_LD1+i, (char*)pgm_read_word(&(nx_pwm_names[i])));
                //delay(1);
                setText (PAGE_PWM_LIST, NX_FIELD_BLD1+i, (char*)pgm_read_word(&(nx_pwm_names[i])));
                //delay(1);
        }
}

void Nextion::keepAlive()
{
        if (currentMillis - __last_keepAlive > NEXTION_KEEP_ALIVE_PING)
        {
                __last_keepAlive = currentMillis;
                startCommand (NX_FIELD_EMPTY, NX_FIELD_SENDME, NX_CMD_EMPTY,  false, false, false);
                endCommand(false);

        }
        if (currentMillis - __last_response > NEXTION_KEEP_ALIVE)
        {
                __last_response = currentMillis;
                reconnect();
        }
}


// main touch listener
void Nextion::listen()
{
        while (NEXTION_AVAIL ()> 0)
        {

                char c = NEXTION_READ ();

                if (c == NEX_RET_EVENT_TOUCH_HEAD || c == NEX_RET_EVENT_PAGE_HEAD)
                {
                        memset(__nx_buffer, 0, sizeof (__nx_buffer));
                        __command_start = true;
                        lastCommand = {};
                        lastCommand.command = c;
                        memset(lastCommand.value, 0, sizeof (lastCommand.value));
                        continue;
                }
                if (__command_start)
                {
                        __nx_buffer[__nx_buffer_ix] = c;
                        __nx_buffer_ix++;

                        if (__nx_buffer_ix >= 3 )
                        {
                                if (memcmp(__nx_buffer + (__nx_buffer_ix-3), nextionEol, 3) ==0)
                                {
                                        __command_complete = true;
                                        memcpy(&lastCommand.value, &__nx_buffer, __nx_buffer_ix);
                                        __command_start = false;
                                        __nx_buffer_ix=0;
                                        processResponse ();
                                        return;
                                }
                        }
                        if (__nx_buffer_ix == 9)
                        {
                                __command_start = false;
                                __nx_buffer_ix=0;
                                return;
                        }
                }
        }
}

void Nextion::processResponse ()
{
        switch (lastCommand.command)
        {
        case NEX_RET_EVENT_TOUCH_HEAD:
                handlePage ( lastCommand.value [0], lastCommand.value [1] );
                break;

        case NEX_RET_EVENT_PAGE_HEAD:
                __last_response = currentMillis;
                break;


        default:
                return;
        }
}



void Nextion::handlePage (byte pid, byte cid)
{
        lastTouch = currentMillis;
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

        case PAGE_ERROR:
                handleScreenSaver (cid);
                break;

        case PAGE_PWMSTATUS:
                handlePWMStatus (cid);
                break;

        default:
                break;
        }
        lastTouch = currentMillis;
}

void Nextion::handleScreenSaver (byte cid)
{
        forceRefresh = true;
        setPage (PAGE_HOME);
        lastTouch = currentMillis;
        toggleButtons();
        refreshPWMNames ();
        refreshHomePage ();
}

boolean Nextion::setThermo (byte page, byte field, byte i)
{
        if (!getNumber(page, field, &t)) return false;
        for (byte k = 0; k < 8; k++)
                settings.sensors[i][k] = __sensors->getList(t).address[k];
        return true;
}

void Nextion::handleThermoPage (byte cid)
{
        boolean status = false;
        switch (cid)
        {
        // save
        case THERMOPAGE_BUTTON_SAVE:
                setPage (PAGE_SAVING);
                if (!setThermo (PAGE_THERMO, NX_FIELD_N0, LED_TEMPERATURE_FAN)) break;
                if (!setThermo (PAGE_THERMO, NX_FIELD_N1, SUMP_TEMPERATURE_FAN)) break;
                if (!setThermo (PAGE_THERMO, NX_FIELD_N2, WATER_TEMPERATURE_FAN)) break;
                __dataStorage->writeEEPROMSettings ();
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                status = true;
                break;

        //cancel
        case THERMOPAGE_BUTTON_CANCEL:
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                status = true;
                break;

        default:
                break;
        }
        if (!status) setPage (PAGE_THERMO);
}

void Nextion::handleSchedulePage (byte cid)
{
        switch (cid)
        {
        //cancel
        case SCHEDULEPAGE_BUTTON_CLOSE:
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                break;

        default:
                break;
        }
}

boolean Nextion::handleTestSlider (int field, byte i)
{
        if (!getNumber(field, &t)) return false;
        //pwm.getRuntime(i).valueCurrent = mapRound ((long)t, 0, 100, 0, PWM_I2C_MAX);
        //pwm.getRuntime(i).testMode = true;


        pwm.setCurrentValue (i, __pwm->mapRound ((long)t, 0, 100, 0, PWM_I2C_MAX));
        pwm.setTestMode (i, true);
        return true;
}

void Nextion::handleTestPage (byte cid)
{
        switch (cid)
        {
        // pwms
        case TESTPAGE_SLIDER_PWM_1:
                handleTestSlider (NX_FIELD_N0, 0);
                break;
        case TESTPAGE_SLIDER_PWM_2:
                handleTestSlider (NX_FIELD_N1, 1);
                break;
        case TESTPAGE_SLIDER_PWM_3:
                handleTestSlider (NX_FIELD_N2, 2);
                break;
        case TESTPAGE_SLIDER_PWM_4:
                handleTestSlider (NX_FIELD_N3, 3);
                break;
        case TESTPAGE_SLIDER_PWM_5:
                handleTestSlider (NX_FIELD_N4, 4);
                break;
        case TESTPAGE_SLIDER_PWM_6:
                handleTestSlider (NX_FIELD_N5, 5);
                break;
        case TESTPAGE_SLIDER_PWM_7:
                handleTestSlider (NX_FIELD_N6, 6);
                break;
        case TESTPAGE_SLIDER_PWM_8:
                handleTestSlider (NX_FIELD_N7, 7);
                break;

        // close
        case TESTPAGE_BUTTON_CLOSE:
                lastTouch = currentMillis;
                pwm.forcePWMRecovery  (true);
                for (byte i = 0; i < PWMS; i++)
                {
                        //pwm.getRuntime(i).valueTest = 0;
                        pwm.setTestMode (i, false);

                }
                setPage (PAGE_CONFIG);
                break;

        default:
                break;
        }
}

void Nextion::handlePWMPage (byte cid)
{
        byte i;
        byte lastPin;
        byte lastI2C;
        boolean status = false;
        switch (cid)
        {
        // save
        case PWMPAGE_BUTTON_SAVE:
                setPage (PAGE_SAVING);
                if (!getNumber(PAGE_PWM, NX_FIELD_N9, &i)) break;
                if (i < 1 || i > PWMS) break;
                lastPin = pwmSettings[i - 1].pin;
                lastI2C = pwmSettings[i - 1].isI2C;

                if (!getNumber(PAGE_PWM, NX_FIELD_C0, &pwmSettings[i - 1].enabled )) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_C1, &pwmSettings[i - 1].isNightLight)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_C2, &pwmSettings[i - 1].isProg)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N0, &pwmSettings[i - 1].onHour)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N1, &pwmSettings[i - 1].onMinute)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N2, &pwmSettings[i - 1].offHour)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N3, &pwmSettings[i - 1].offMinute)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N4, &pwmSettings[i - 1].sunriseLenght)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N5, &pwmSettings[i - 1].sunsetLenght)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N6, &pwmSettings[i - 1].valueNight)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N7, &pwmSettings[i - 1].valueDay)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N8, &pwmSettings[i - 1].valueProg)) return;
                if (!getNumber(PAGE_PWM, NX_FIELD_N10, &pwmSettings[i - 1].pin)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_N11, &pwmSettings[i - 1].watts)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_C3, &pwmSettings[i - 1].isI2C)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_C4, &pwmSettings[i - 1].invertPwm)) break;
                if (!getNumber(PAGE_PWM, NX_FIELD_C5, &pwmSettings[i - 1].useLunarPhase)) break;

                if (lastPin != pwmSettings[i - 1].pin || lastI2C != pwmSettings[i - 1].isI2C) pwm.initPWM ( i-1 );

                //pwmSettings[i - 1].valueNight = mapRound ((byte)pwmSettings[i - 1].valueNight, 0, 255, 0, PWM_I2C_MAX);
                pwmSettings[i - 1].valueProg = __pwm->mapRound (pwmSettings[i - 1].valueProg, 0, 100, 0, PWM_I2C_MAX);
                pwmSettings[i - 1].valueDay = __pwm->mapRound (pwmSettings[i - 1].valueDay, 0, 100, 0, PWM_I2C_MAX);
                lastTouch = currentMillis;
                __dataStorage->writeEEPROMPWMConfig (i - 1);
                pwm.updateChannelTimes  (i-1);
                setPage (PAGE_PWM_LIST);
                status = true;
                break;

        //  cancel
        case PWMPAGE_BUTTON_CANCEL:
                lastTouch = currentMillis;
                setPage (PAGE_PWM_LIST);
                status = true;
                break;

        default:
                status = true;
                break;
        }
        if (!status) setPage (PAGE_PWM_LIST);
}

void Nextion::handleSettingsPage (byte cid)
{
        boolean status = false;
        switch (cid)
        {
        // save
        case SETTINGSPAGE_BUTTON_SAVE:
                setPage (PAGE_SAVING);
                if (!getNumber(PAGE_SETTINGS,NX_FIELD_N0, &settings.maxTemperatures[LED_TEMPERATURE_FAN] )) break;
                if (!getNumber(PAGE_SETTINGS,NX_FIELD_N1, &settings.maxTemperatures[SUMP_TEMPERATURE_FAN])) break;
                if (!getNumber(PAGE_SETTINGS,NX_FIELD_N2, &settings.maxTemperatures[WATER_TEMPERATURE_FAN])) break;
                if (!getNumber(PAGE_SETTINGS,NX_FIELD_N3, &settings.pwmDimmingTime)) break;
                if (!getNumber(PAGE_SETTINGS,NX_FIELD_N4, &settings.screenSaverTime)) break;
                if (!getNumber(PAGE_SETTINGS,NX_FIELD_C0, &settings.softDimming)) break;
                __dataStorage->writeEEPROMSettings ();
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                status = true;
                break;

        // cancel
        case SETTINGSPAGE_BUTTON_CANCEL:
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                status = true;
                break;

        default:
                break;
        }
        if (!status) setPage (PAGE_SETTINGS);
}

void Nextion::handleSetTimePage (byte cid)
{
        byte setHour, setMinute,  setMonth, setDay;
        int setYear;
        boolean status = false;
        switch (cid)
        {
        // save
        case TIMEPAGE_BUTTON_SAVE:
                setPage (PAGE_SAVING);
                if (!getNumber(PAGE_SETTIME, NX_FIELD_N0, &setHour)) break;
                if (!getNumber(PAGE_SETTIME, NX_FIELD_N1, &setMinute)) break;
                if (!getNumber(PAGE_SETTIME, NX_FIELD_N2, &setDay)) break;
                if (!getNumber(PAGE_SETTIME, NX_FIELD_N3, &setMonth)) break;
                if (!getNumber(PAGE_SETTIME, NX_FIELD_N4, &setYear)) break;
                if (!getNumber(PAGE_SETTIME, NX_FIELD_C0, &settings.dst)) break;
                time.adjust(DateTime(setYear, setMonth, setDay, setHour, setMinute,0));
                time.read ();
                time.adjustDST ();
                __dataStorage->writeEEPROMSettings ();
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                lastTouch = currentMillis;
                pwm.forcePWMRecovery  (false);
                status = true;
                break;

        // cancel
        case TIMEPAGE_BUTTON_CANCEL:
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                lastTouch = currentMillis;
                status = true;
                break;

        default:
                status = true;
                break;
        }
        if (!status) setPage (PAGE_SETTIME);
}

void Nextion::drawSchedule ()
{
        int startL;
        int min_start;
        int min_stop;
        int stopL;
        int stopM;
        int min_sunrise;
        int min_sunrise_left;
        int min_sunset;
        int min_sunset_left;
        boolean midnight = false;

        for (byte i = 0; i < PWMS; i++)
        {
                min_start = (int)pwmSettings[i].onHour * (int)60 + (int)pwmSettings[i].onMinute;
                min_stop = (int)pwmSettings[i].offHour * (int)60 + (int)pwmSettings[i].offMinute;

                // background
                if (pwmSettings[i].isNightLight == 1)
                        fillRect (offset * i + startx, starty, width, height, COLOR_DARKBLUE);
                else
                        fillRect (offset * i + startx, starty, width, height, COLOR_RED);

                // off
                if (pwmSettings[i].enabled == 0) continue;

                // light
                if (min_stop < min_start) midnight = true;
                else midnight = false;

                if (!midnight)
                {
                        startL = map (min_start, 0, min_hour, starty, starty + height);
                        stopL = map (min_stop, 0, min_hour, starty, starty + height ) - startL;
                        fillRect (offset * i + startx, startL, width, stopL, COLOR_GREEN);
                }
                else
                {
                        startL = map (min_start, 0, min_hour, starty, starty + height);
                        stopL = map (min_stop, 0, min_hour, starty, starty + height ) - 30;
                        stopM =  map (min_hour, 0, min_hour, starty, starty + height ) - startL;
                        // start to midnight
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_GREEN);
                        // midnight to end
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_GREEN);
                }
                // sunrise
                min_sunrise  = min_start + (int)pwmSettings[i].sunriseLenght;
                if (min_sunrise > min_hour) midnight = true; else midnight = false;

                if (!midnight)
                {
                        startL = map (min_start, 0, min_hour, starty, starty + height);
                        stopL = map (min_sunrise, 0, min_hour, starty, starty + height ) - startL;
                        fillRect (offset * i + startx, startL, width, stopL, COLOR_BLUE);
                }
                else
                {
                        startL = map (min_start, 0, min_hour, starty, starty + height);
                        stopM =  map (min_hour, 0, min_hour, starty, starty + height ) - startL;
                        min_sunrise_left = min_sunrise - min_hour;
                        stopL = map (min_sunrise_left, 0, min_hour, starty, starty + height ) - starty;
                        // start to midnight
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_BLUE);
                        // midnight to end
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_BLUE);
                }
                // sunset
                min_sunset  = min_stop - (int)pwmSettings[i].sunsetLenght;
                if (min_sunset < 0) midnight = true; else midnight = false;
                if (!midnight)
                {
                        startL = map (min_sunset, 0, min_hour, starty, starty + height);
                        stopL = map (min_stop, 0, min_hour, starty, starty + height ) - startL;
                        fillRect (offset * i + startx, startL, width, stopL, COLOR_BLUE);
                }
                else
                {
                        min_sunset_left = min_hour + min_sunset;
                        stopL  = map (min_stop, 0, min_hour, starty, starty + height ) - starty;
                        startL = map (min_sunset_left, 0, min_hour, starty, starty + height);
                        stopM =  map (min_hour, 0, min_hour, starty, starty + height ) - startL;
                        // zero to end
                        fillRect (offset * i + startx, starty, width, stopL, COLOR_BLUE);
                        // rest till midnight
                        fillRect (offset * i + startx, startL, width, stopM, COLOR_BLUE);
                }
        }
        int min_now = map (__time->getHour() * 60 + __time->getMinute(), 0, min_hour, starty, starty + height);
        fillRect (hour_startx, min_now, hour_stopx, 1, COLOR_YELLOW);
        // siatka dodatkowa
        /*
           for (int i = 0 ; i < 24; i++)
           {
           fillRect (40, 10 * i + starty+1, 180, 1, COLOR_DARKGRAY);

           }*/
}

void Nextion::handleConfigPage (byte cid)
{
        byte s;
        byte idxLed, idxSump, idxWater;
        char tempbuff[50] = {0};

        switch (cid)
        {
        // schedule
        case CONFIGPAGE_BUTTON_SCHEDULE:
                setPage (PAGE_SCHEDULE);
                drawSchedule ();
                break;

        // close
        case CONFIGPAGE_BUTTON_CLOSE:
                forceRefresh = true;
                setPage (PAGE_HOME);
                toggleButtons();
                refreshHomePage ();
                break;

        // thermo setup
        case CONFIGPAGE_BUTTON_THERMO:
              #ifndef NO_TEMPERATURE
                setPage (PAGE_THERMO);
                s = __sensors->discoverOneWireDevices ();
                memset (tempbuff, 0, sizeof (tempbuff));
                setValue (NX_FIELD_VA0, (s-1));
                for (byte i = 0; i < s; i++)
                {
                        sprintf(tempbuff + strlen (tempbuff), "%d. ", i);
                        if (i == 0)
                        {
                                strcpy (tempbuff + strlen (tempbuff), "BRAK ");
                        }
                        else
                        {
                                if (__sensors->getList(i).detected)
                                        strcpy (tempbuff + strlen (tempbuff), "* ");
                                for (byte k = 0; k < 8; k++)
                                        sprintf (tempbuff + strlen (tempbuff), "%02x ", __sensors->getList(i).address[k]);
                        }
                        strcpy  (tempbuff + strlen (tempbuff), "\r\n");
                }
                idxLed = __sensors->listContains (settings.sensors[LED_TEMPERATURE_FAN]);
                idxSump = __sensors->listContains (settings.sensors[SUMP_TEMPERATURE_FAN]);
                idxWater = __sensors->listContains (settings.sensors[WATER_TEMPERATURE_FAN]);
                if (idxLed == 255) idxLed = 0;
                if (idxSump == 255) idxSump = 0;
                if (idxWater == 255) idxWater = 0;
                setValue (NX_FIELD_N0, idxLed);
                setValue (NX_FIELD_N1, idxSump);
                setValue (NX_FIELD_N2, idxWater);
                setText (NX_FIELD_T4, (String) tempbuff);
              #endif
                break;

        // test
        case CONFIGPAGE_BUTTON_TEST:
                setPage (PAGE_TEST);
                for (byte i = 0; i < PWMS; i++)
                {
                        setValue (NX_FIELD_N0+i, __pwm->mapRound (pwm.getRuntime(i).valueCurrent, 0, PWM_I2C_MAX, 0, 100));
                        setValue (NX_FIELD_C1+i, __pwm->mapRound (pwm.getRuntime(i).valueCurrent, 0, PWM_I2C_MAX, 0, 100));
                        //pwm.getRuntime(i).valueTest = pwm.getRuntime(i).valueCurrent;
                        pwm.setTestMode(i, false);
                }
                break;

        // hour and date setup
        case CONFIGPAGE_BUTTON_TIME:
                setPage (PAGE_SETTIME);
                setValue (NX_FIELD_N0, __time->getHour ());
                setValue (NX_FIELD_N1, __time->getMinute ());
                setValue (NX_FIELD_N2, __time->getDay());
                setValue (NX_FIELD_N3, __time->getMonth ());
                setValue (NX_FIELD_N4, __time->getYear());
                setValue (NX_FIELD_C0, settings.dst);
                break;

        // other settings
        case CONFIGPAGE_BUTTON_SETTINGS:
                setPage (PAGE_SETTINGS);
                setValue (NX_FIELD_N0, settings.maxTemperatures[LED_TEMPERATURE_FAN]);
                setValue (NX_FIELD_N1, settings.maxTemperatures[SUMP_TEMPERATURE_FAN]);
                setValue (NX_FIELD_N2, settings.maxTemperatures[WATER_TEMPERATURE_FAN]);
                setValue (NX_FIELD_N3, settings.pwmDimmingTime);
                setValue (NX_FIELD_N4, settings.screenSaverTime);
                setValue (NX_FIELD_C0, settings.softDimming);
                break;

        // pwm config
        case CONFIGPAGE_BUTTON_PWM:
                setPage (PAGE_PWM_LIST);
                break;

        default:
                break;
        }
}


void Nextion::handlePWMStatus (byte cid)
{

        switch (cid)
        {
        // cancel
        case PWMSTATUSPAGE_BUTTON_CLOSE:
                lastTouch = currentMillis;
                setPage (PAGE_HOME);
                toggleButtons();
                refreshHomePage ();
                lastTouch = currentMillis;
                break;

        default:
                break;
        }
}

void Nextion::handlePWMListPage (byte cid)
{
        byte tmin, tmax, tamb;

        switch (cid)
        {
        // enter pwm settings
        case PWMCONFIGPAGE_BUTTON_PWM_1:
        case PWMCONFIGPAGE_BUTTON_PWM_2:
        case PWMCONFIGPAGE_BUTTON_PWM_3:
        case PWMCONFIGPAGE_BUTTON_PWM_4:
        case PWMCONFIGPAGE_BUTTON_PWM_5:
        case PWMCONFIGPAGE_BUTTON_PWM_6:
        case PWMCONFIGPAGE_BUTTON_PWM_7:
        case PWMCONFIGPAGE_BUTTON_PWM_8:
                tmin = pwmSettings[cid - 1].valueNight;
                tmax = (byte) __pwm->mapRound (pwmSettings[cid - 1].valueDay, 0, PWM_I2C_MAX, 0, 100);
                tamb = (byte) __pwm->mapRound (pwmSettings[cid - 1].valueProg, 0, PWM_I2C_MAX, 0, 100);
                setPage (PAGE_PWM);
                setValue (NX_FIELD_C0, pwmSettings[cid - 1].enabled);
                setValue (NX_FIELD_C1, pwmSettings[cid - 1].isNightLight);
                setValue (NX_FIELD_C2, pwmSettings[cid - 1].isProg);
                setValue (NX_FIELD_N0, pwmSettings[cid - 1].onHour);
                setValue (NX_FIELD_N1, pwmSettings[cid - 1].onMinute);
                setValue (NX_FIELD_N2, pwmSettings[cid - 1].offHour);
                setValue (NX_FIELD_N3, pwmSettings[cid - 1].offMinute);
                setValue (NX_FIELD_N4, pwmSettings[cid - 1].sunriseLenght);
                setValue (NX_FIELD_N5, pwmSettings[cid - 1].sunsetLenght);
                setValue (NX_FIELD_N6, tmin);
                setValue (NX_FIELD_N7, tmax);
                setValue (NX_FIELD_N8, tamb);
                setValue (NX_FIELD_N9, cid);
                setValue (NX_FIELD_N10, pwmSettings[cid - 1].pin);
                setValue (NX_FIELD_N11, pwmSettings[cid - 1].watts);
                setValue (NX_FIELD_C3, pwmSettings[cid - 1].isI2C);
                setValue (NX_FIELD_C4, pwmSettings[cid - 1].invertPwm);
                setValue (NX_FIELD_C5, pwmSettings[cid - 1].useLunarPhase);
                setText (NX_FIELD_T4,  (char*)pgm_read_word(&(nx_pwm_names[cid -1])));
                break;

        // cancel
        case PWMCONFIGPAGE_BUTTON_CLOSE:
                lastTouch = currentMillis;
                setPage (PAGE_CONFIG);
                lastTouch = currentMillis;
                break;

        default:
                break;
        }
}

void Nextion::handleHomePage (byte cid)
{

        switch (cid)
        {

        // config show
        case HOMEPAGE_BUTTON_CONFIG:
                setPage (PAGE_CONFIG);
                break;

        //toggle night mode
        case HOMEPAGE_BUTTON_NIGHT:
                if (settings.forceAmbient  == 1 || settings.forceOFF  == 1) return;
                if (settings.forceNight == 1)
                {
                        settings.forceNight = 0;
                        pwm.forcePWMRecovery  (false);
                }
                else
                {
                        settings.forceNight = 1;
                        pwm.forceDimmingRestart ();
                }

                toggleButtons();
                __dataStorage->writeEEPROMForceNight ();
                break;

        // ambient toggle
        case HOMEPAGE_BUTTON_AMBIENT:
                if (settings.forceOFF == 1 || settings.forceNight == 1) return;
                if (settings.forceAmbient == 1)
                {
                        settings.forceAmbient = 0;
                        pwm.forcePWMRecovery  (false);
                }
                else
                {
                        settings.forceAmbient = 1;
                        pwm.forceDimmingRestart ();
                }

                toggleButtons();
                __dataStorage->writeEEPROMForceAmbient ();
                break;

        // off/on toggle
        case HOMEPAGE_BUTTON_OFF:
                if (settings.forceOFF == 1)
                {
                        settings.forceOFF = 0;
                        pwm.forcePWMRecovery  (false);
                }
                else
                {
                        settings.forceOFF = 1;
                        pwm.forceDimmingRestart ();
                }
                toggleButtons();
                __dataStorage->writeEEPROMForceOff ();
                break;

        // fans toggle
        case HOMEPAGE_BUTTON_FAN_WATER:
                __sensors->invertFan (WATER_TEMPERATURE_FAN, true);
                updateHomePage ();
                break;

        case HOMEPAGE_BUTTON_FAN_LAMP:
                __sensors->invertFan (LED_TEMPERATURE_FAN, true);
                updateHomePage ();
                break;

        case HOMEPAGE_BUTTON_FAN_SUMP:
                __sensors->invertFan (SUMP_TEMPERATURE_FAN, true);
                updateHomePage ();
                break;

        case HOMEPAGE_PWMSTATUS1:
        case HOMEPAGE_PWMSTATUS2:
        case HOMEPAGE_PWMSTATUS3:
        case HOMEPAGE_PWMSTATUS4:
        case HOMEPAGE_PWMSTATUS5:
        case HOMEPAGE_PWMSTATUS6:
        case HOMEPAGE_PWMSTATUS7:
        case HOMEPAGE_PWMSTATUS8:
                activePwmStatus = cid - HOMEPAGE_PWMSTATUS1;
                setPage (PAGE_PWMSTATUS);
                updatePWMStatusPage (activePwmStatus);
                break;

        default:
                break;
        }
}

void Nextion::toggleButton (byte value, byte field, byte pic_on, byte pic_off)
{
        if (value == 1) setInt(field,NX_CMD_PIC, pic_on); else setInt(field,NX_CMD_PIC,  pic_off);
}

// toggle home page buttons images
void Nextion::toggleButtons()
{
        toggleButton (settings.forceOFF, NX_FIELD_BO, NX_PIC_BO_ON, NX_PIC_BO_OFF);
        toggleButton (settings.forceAmbient, NX_FIELD_BA, NX_PIC_BA_ON, NX_PIC_BA_OFF);
        toggleButton (settings.forceNight, NX_FIELD_BN, NX_PIC_BN_ON, NX_PIC_BN_OFF);
}

#ifndef NO_TEMPERATURE
void Nextion::updateTempField (byte field, byte sensor, byte max, byte min)
{
        if (__sensors->getConfig(sensor).nxTemperature != __sensors->getConfig(sensor).temperature  || forceRefresh)
        {
                if (settings.sensors[sensor][0]==0)
                {
                        setText(field, NX_STR_DASH);
                        setInt (field, NX_CMD_PCO, COLOR_DARKGRAY);
                }else

                if (__sensors->getConfig(sensor).temperature != TEMP_ERROR)
                {
                        setTextFloat(field, __sensors->getConfig(sensor).temperature,1,NX_STR_DEGREE);

                        __sensors->setNXTemperature (sensor, __sensors->getConfig(sensor).temperature);
                        if (__sensors->getConfig(sensor).temperature < min || __sensors->getConfig(sensor).temperature > max)
                                setInt (field, NX_CMD_PCO, COLOR_LIGHTRED);
                        else
                                setInt (field, NX_CMD_PCO,  COLOR_LIGHTGREEN);
                }
                else if (__sensors->getConfig(sensor).temperature == TEMP_ERROR)
                {
                        setText(field, NX_STR_ERR);
                        setInt (field, NX_CMD_PCO, COLOR_RED);
                }

                if (settings.sensors[sensor][0]==0)
                {
                        setText(field, NX_STR_DASH);
                        setInt (field, NX_CMD_PCO, COLOR_DARKGRAY);
                }

        }
}

void Nextion::updateFanField (byte field, byte sensor)
{
        if (__sensors->getConfig(sensor).nxFanStatus == __sensors->getConfig(sensor).fanStatus) return;
        if (!__sensors->getConfig(sensor).fanStatus) setText (field, NX_STR_EMPTY);
        else setText (field, NX_STR_FAN);
        __sensors->setNXFanStatus(sensor, __sensors->getConfig(sensor).fanStatus);

}
#endif

void Nextion::updateWaterTemp()
{
    #ifndef NO_TEMPERATURE
        updateTempField (NX_FIELD_WT, WATER_TEMPERATURE_FAN,settings.maxTemperatures[WATER_TEMPERATURE_FAN], WATER_TEMPERATURE_MIN);
    #endif
}


void Nextion::updatePWMStatusPage (byte i)
{
        uint16_t color;
        byte icon;
        setText (NX_FIELD_EMPTY, NX_FIELD_T0, (char*)pgm_read_word(&(nx_pwm_names[i])));
        getColorAndIcon (i, &color, &icon);
        setTextFloat (NX_FIELD_T1,  getPercent (i), 1, NX_STR_PERCENT);
        setTextInt (NX_FIELD_T2, pwm.getRuntime(i).valueCurrent);
        setText (NX_FIELD_T3, icon);
        setInt (NX_FIELD_T3, NX_CMD_PCO,  color);
        setTextInt (NX_FIELD_T4, pwm.getRuntime(i).secondsLeft);
        setTextInt (NX_FIELD_T5, pwm.getRuntime(i).valueGoal);
        setTextFloat (NX_FIELD_T6, pwm.getRuntime(i).watts, 1, NX_STR_WATTS);
        setTextFloat (NX_FIELD_T7, pwm.getRuntime(i).step,3, NX_FIELD_EMPTY);
        setTextFloat (NX_FIELD_T9, moonPhases[moonPhase],0,NX_STR_PERCENT);
        char buf[12];
        uint32_t uptime  = __time->getUnixTime() - startTimestamp;
        memset (buf, 0, sizeof (buf));
        long mins=uptime/60;
        long hours=mins/60;
        long days=hours/24;
        mins=mins-(hours*60);
        hours=hours-(days*24);
        sprintf (buf, "%lud %02lu:%02lu",days, hours,mins);
        setText (NX_FIELD_T8, String (buf));
}


void Nextion::updateHomePage()
{
        #ifndef NO_TEMPERATURE
        updateWaterTemp();
        updateTempField (NX_FIELD_LT, LED_TEMPERATURE_FAN,settings.maxTemperatures[LED_TEMPERATURE_FAN], 0);
        updateTempField (NX_FIELD_ST, SUMP_TEMPERATURE_FAN,settings.maxTemperatures[SUMP_TEMPERATURE_FAN], 0);
        #endif
        for (byte i = 0; i < PWMS; i++)
        {
                byte valueField = 66+i;
                byte iconField =  3+i;
                if (!pwm.getRuntime(i).valueCurrent && pwmSettings[i].enabled == 0 && (pwm.getRuntime(i).nxPwmLast != 0 || forceRefresh))
                {
                        setText (valueField, NX_STR_DASH);
                        setText (iconField, NX_STR_SPACE);
                        pwm.setnxPwmLast(i,0);
                        //  pwm.getRuntime(i).nxPwmLast = 0;
                        continue;
                }

                if (pwm.getRuntime(i).nxPwmLast!=pwm.getRuntime(i).valueCurrent || forceRefresh)
                {
                        uint16_t color;
                        byte icon;
                        getColorAndIcon (i, &color, &icon);
                        pwm.setnxPwmLast(i,pwm.getRuntime(i).valueCurrent);
                        setTextFloat (valueField,  getPercent (i), 1, NX_STR_PERCENT);
                        setText (iconField, icon);
                        setInt (iconField, NX_CMD_PCO,  color);
                }
        }
        #ifndef NO_TEMPERATURE
        updateFanField (NX_FIELD_T0, WATER_TEMPERATURE_FAN);
        updateFanField (NX_FIELD_T1, LED_TEMPERATURE_FAN);
        updateFanField (NX_FIELD_T2, SUMP_TEMPERATURE_FAN);
        #endif
}

double Nextion::getPercent (byte i)
{
        return __pwm->mapDouble((double)pwm.getRuntime(i).valueCurrent, 0.0, (double)PWM_I2C_MAX, 0.0, 100.0);
}

void Nextion::getColorAndIcon (byte i, uint16_t *color, byte *icon)
{
        *color = COLOR_WHITE;
        *icon = NX_STR_SPACE;
        if (pwm.getRuntime(i).isSunrise )
        {
                *icon = NX_STR_SUNRISE;
                *color = COLOR_LIGHTYELLOW;
        }
        else if (pwm.getRuntime(i).isSunset )
        {
                *icon = NX_STR_SUNSET;
                *color = COLOR_ORANGE;
        }
        else if (pwm.getRuntime(i).recoverLastState)
        {
                *icon = NX_STR_RECOVER;
                *color = COLOR_LIGHTGREEN;
        }
        else if (pwm.getRuntime(i).valueCurrent < pwm.getRuntime(i).valueGoal)
        {
                *icon = NX_STR_UP;

        }
        else if (pwm.getRuntime(i).valueCurrent > pwm.getRuntime(i).valueGoal) *icon = NX_STR_DOWN;
        else if (pwm.getRuntime(i).valueCurrent == 0 || pwmSettings[i].enabled == 0)
        {
                *icon = NX_STR_OFF;
                *color = COLOR_LIGHTRED;
        }
        else if (pwm.getRuntime(i).isNight)
        {
                if (pwmSettings[i].useLunarPhase)
                {
                        *color = rgb565 (map (moonPhases[moonPhase], 0,100,150,255));
                }
                *icon = NX_STR_NIGHT;
        }
        else if (pwm.getRuntime(i).valueCurrent == pwmSettings[i].valueDay)
        {
                *icon = NX_STR_ON;
                *color = COLOR_YELLOW;
        }
}

uint16_t Nextion::rgb565( byte rgb)
{
        uint16_t ret  = (rgb & 0xF8) << 8;// 5 bits
        ret |= (rgb & 0xFC) << 3;       // 6 bits
        ret |= (rgb & 0xF8) >> 3;       // 5 bits
        return( ret);
}

void Nextion::displayWats ()
{
        if(watts!=lastWatts || forceRefresh) setTextFloat (NX_FIELD_WA, watts, 1, NX_STR_WATTS);
        lastWatts= watts;
}

void Nextion::timeDisplay()
{
        char buff[7] = {0};
        memset(buff, 0, sizeof (buff));
        if (time_separator % 2 == 0)
                sprintf(buff + strlen(buff), "%02u:%02u", __time->getHour(), __time->getMinute());
        else sprintf(buff + strlen(buff), "%02u %02u", __time->getHour(), __time->getMinute());
        setText (NX_FIELD_H, (String)buff);
        time_separator++;
}

void Nextion::refreshHomePage ()
{
        timeDisplay();
        updateHomePage();
        displayWats ();

}

/* void Nextion::displayMemory ()
   {
        double freemem = ( (double) getFreeMemory() / (double) 2048) *100;
        setTextFloat (NX_FIELD_DEBUG2, freemem,  1, NX_STR_PERCENT);
   }*/
/*
    void Nextion::nxSetDebug ()
   {
    if (nxScreen == PAGE_SCREENSAVER )
    {
        setTextInt (NX_FIELD_DEBUG1, codePoint);

    }
   }
 */

void Nextion::display ()
{
        if (currentMillis - previousNxInfo > NX_INFO_RESOLUTION)
        {
                previousNxInfo = currentMillis;
                if (lampOverheating)
                {
                        nxScreen = PAGE_ERROR;
                        setPage (PAGE_ERROR);
                        setText (NX_FIELD_T1, (char*)pgm_read_word(&(nx_errors[0])) );
                }
                if (max_watts_exceeded)
                {
                        nxScreen = PAGE_ERROR;
                        setPage (PAGE_ERROR);
                        setText (NX_FIELD_T1, (char*)pgm_read_word(&(nx_errors[1])) );
                }
                if (nxScreen == PAGE_SCREENSAVER )
                {
                        timeDisplay();
                        updateWaterTemp();
                        displayWats ();
                        //  displayMemory ();
                        forceRefresh = false;
                }
                if (nxScreen == PAGE_HOME )
                {
                        if (forceRefresh) refreshPWMNames ();
                        refreshHomePage ();
                        forceRefresh = false;
                }
                if (nxScreen == PAGE_PWMSTATUS)
                {
                        updatePWMStatusPage(activePwmStatus);
                }
                // screensaver
                if (currentMillis - lastTouch > settings.screenSaverTime*1000 && nxScreen != PAGE_SCREENSAVER && settings.screenSaverTime > 0 && nxScreen == PAGE_HOME )
                {
                        setPage (PAGE_SCREENSAVER);
                        setText (NX_FIELD_T1,NX_STR_EMPTY);
                        forceRefresh = true;
                }
        }
        if (lastTouch == 0) lastTouch = currentMillis;

}

/* NEXTION COMMUNICATION */

void Nextion::sendNextionEOL ()
{
        NEXTION_WRITEB(nextionEol, 3);
}

void Nextion::startCommand  (byte page, byte field, byte command, boolean eq, boolean pth, boolean space)
{
        if (page!=NX_FIELD_EMPTY)
        {
                printPGM( (char*)pgm_read_word(&(nx_pages[page])));
                printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_DOT])));
        }
        if (field!=NX_FIELD_EMPTY) printPGM( (char*)pgm_read_word(&(nx_fields[field])));
        if (command!=NX_CMD_EMPTY) printPGM( (char*)pgm_read_word(&(nx_commands[command])));
        if (eq) printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_EQ])));
        else if (space) printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_SPACE])));
        if (pth) printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_PARENTH])));
}

void Nextion::endCommand  (boolean pth)
{
        if (pth) printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_PARENTH])));
        sendNextionEOL ();
}

void Nextion::setInt (byte field, long cmd, long val)
{
        startCommand (NX_FIELD_EMPTY, field, cmd,  true, false, true);
        NEXTION_PRINT(val);
        endCommand (false);
}



void Nextion::setTextFloat (byte field, double txt, byte prec, byte str)
{
        startCommand (NX_FIELD_EMPTY, field, NX_CMD_TXT, true,  true, true);
        NEXTION_PRINTF( txt, prec );
        if (str!=NX_FIELD_EMPTY) printPGM( (char*)pgm_read_word(&(nx_strings[str])));
        endCommand (true);
}
void Nextion::setValue (byte field, unsigned int val)
{
        startCommand (NX_FIELD_EMPTY, field, NX_CMD_VAL,  true, false, true);
        NEXTION_PRINT(val);
        endCommand (false);
}

void Nextion::setTextInt (byte field, int txt)
{
        startCommand (NX_FIELD_EMPTY, field, NX_CMD_TXT, true,  true, true);
        NEXTION_PRINT( txt );
        endCommand (true);
}

void Nextion::setPage (byte number)
{
        startCommand (NX_FIELD_EMPTY, NX_FIELD_EMPTY, NX_CMD_PAGE, false, false, true);
        NEXTION_PRINT(number);
        endCommand (false);
        nxScreen = number;
}

void Nextion::setText (byte field, int nx_string_id)
{
        setText (field, (char*)pgm_read_word(&(nx_strings[nx_string_id])));
}

void Nextion::setText (byte field, String text)
{
        startCommand (NX_FIELD_EMPTY, field, NX_CMD_TXT,  true, true, true);
        //NEXTION_PRINT(text);
        writeString (text);
        endCommand (true);
}

void Nextion::setText (byte page, byte field, const char * text)
{
        startCommand (page, field, NX_CMD_TXT, true,  true, true);
        printPGM(text);
        endCommand (true);
}

void Nextion::setText (byte field, const char * text)
{
        setText (NX_FIELD_EMPTY, field, text);
}

bool Nextion::getNumber (byte field, int *result)
{
        return getNumber (255,  field, result);
}

bool Nextion::getNumber (byte page, byte field, byte *result)
{
        int t;
        if (getNumber (page, field, &t))
        {
                *result = (byte) t;
                return true;
        }
        return false;
}

bool Nextion::getNumber (byte page, byte field, int *result)
{

/*        NEXTION_FLUSH ();
        while(NEXTION_AVAIL() > 0)
        {
                Serial.read();
        }*/
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_GET])));
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_SPACE])));
        if (page!=255)
        {
                printPGM( (char*)pgm_read_word(&(nx_pages[page])));
                printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_DOT])));
        }
        printPGM( (char*)pgm_read_word(&(nx_fields[field])));
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_VAL])));
        sendNextionEOL ();
        uint8_t temp[8] = {0};
        uint32_t r;

        NEXTION_SETTIMEOUT(500);
        if (sizeof(temp) != NEXTION_READBYTES((char *)temp, sizeof(temp)))
        {

                return false;
        }

        if (temp[0] == NEX_RET_NUMBER_HEAD && memcmp( temp+5, nextionEol, 3) == 0)
        {
                r = ((uint32_t)temp[4] << 24) | ((uint32_t)temp[3] << 16) | ((uint32_t)temp[2] << 8) | ((uint32_t)temp[1]);
                *result = (int) r;
                return true;
        }
        return false;
}

// fills nextion rectangle
void Nextion::fillRect (int x, int y, int w, int h, int color)
{

        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_FILL])));
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_SPACE])));
        NEXTION_PRINT (x);
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_COMMA])));
        NEXTION_PRINT (y);
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_COMMA])));
        NEXTION_PRINT (w);
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_COMMA])));
        NEXTION_PRINT (h);
        printPGM( (char*)pgm_read_word(&(nx_commands[NX_CMD_COMMA])));
        NEXTION_PRINT (color);
        sendNextionEOL ();
}

void Nextion::printPGM (const char * str)
{
        if (!str) return;
        char c;
        while ((c = pgm_read_byte(str++)))
                NEXTION_WRITE (c);
}

void Nextion::writeString(String stringData) {  // Used to serially push out a String with Serial.write()
        for (byte i = 0; i < stringData.length(); i++)
        {
                NEXTION_WRITE (stringData[i]); // Push each char 1 by 1 on each loop pass
        }
}

#endif
