

char buffer[30] = {0};

void sendCommandPGMInt (int i,  int8_t b, boolean pth)
{
  memset(buffer, 0, sizeof (buffer));
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[i])));

  sprintf (buffer+strlen (buffer), "%d",b);

  if (pth) strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[1])));
  sendCommand(buffer);
 
}

void sendCommandPGM (int i,  char *b, ...)
{
  memset(buffer, 0, sizeof (buffer));
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[i])));

  va_list ap;
  va_start(ap, b);
  //     char buf[20];
  //     vsnprintf(buf, 20, (const char *)b, ap);

  strcat (buffer, b);
  char* c = va_arg (ap, char*);
  if (c)
    strcat (buffer, c);

  strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[1])));
  sendCommand(buffer);
  va_end(ap);
}




void sendCommandPGM (int i)
{
  memset(buffer, 0, sizeof (buffer));
  strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[i])));
  sendCommand(buffer);

}




void nxTouch()
{
  static uint8_t __buffer[10];
  uint16_t i;
  uint8_t c;
  uint8_t pid, cid;

  while (Serial.available() > 0)
  {
    delay(10);
    c = Serial.read();

    if (NEX_RET_EVENT_TOUCH_HEAD == c)
    {
      if (Serial.available() >= 6)
      {
        __buffer[0] = c;
        for (i = 1; i < 7; i++)
        {
          __buffer[i] = Serial.read();
        }
       
        if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6])
        {
          pid = __buffer[1];
          cid = __buffer[2];
          handlePage (pid, cid);
         // Serial.println ( buffer[1]+ buffer[2]+ buffer[3]+ buffer[4] +buffer[5]);
        }

      }
    }
  }
}

void handlePage (byte pid, byte cid)
{
  switch (pid)
  {
    case 0:
      handleHomePage (cid);
      break;

    case 1:
      handleConfigPage (cid);
      break;
      
     case 2:
      handleSetTimePage (cid);
      break;

    default:
      break;
  }
}

void handleSetTimePage (byte cid)
{
int t;
switch (cid)
  {

    // godzina
    case 6:
       sendCommandPGM (35);
       t = getInt ();
       t++;
       if (t>23) t = 0;
       sendCommandPGMInt (30, 1, true);       
       
      
    break;


    // zapisz date
    case 4:
       sendCommandPGMInt (29, 1, false);
       nxScreen = 1;
    break;

    // wyjdz
    case 5:
       sendCommandPGMInt (29, 1, false);
       nxScreen = 1;
    break;

    default:
      break;
  }


}




void handleConfigPage (byte cid)
{

switch (cid)
  {

    // config show
    case 11:
      sendCommandPGMInt (29, 0, false);
      nxScreen = 0;
      toggleButtons();
    break;


    case 7:
       sendCommandPGMInt (29, 2, false);
       nxsetTime ();
       nxScreen = 2;
    break;

    default:
      break;
  }


}


void handleHomePage (byte cid)
{
  switch (cid)
  {

    // config show
    case 25:
      sendCommandPGMInt (29, 1, false);
      nxScreen = 1;
      break;

    //toggle night mode
    case 24:
      if (forceAmbient  == true || forceOFF  == true) return;
      if (forceNight == true) forceNight = false;  else  forceNight = true;
      toggleButtons();
      break;

    // ambient toggle
    case 26:
      if (forceOFF == true || forceNight == true) return;
      if (forceAmbient == true) forceAmbient = false;  else  forceAmbient = true;
      toggleButtons();
      break;

    // off/on toggle
    case 23:
      if (forceOFF == true) forceOFF = false;  else  forceOFF = true;
      toggleButtons();
      break;

    default:
      break;
  }
}


static void toggleButtons()
{

  if (forceOFF == false)
  {
    sendCommandPGM(2);
    sendCommandPGM(3);
    sendCommandPGM(6);

  }
  else
  {
    sendCommandPGM(4);
    sendCommandPGM(5);
    sendCommandPGM(6);

  }

  if (forceAmbient == false)
  {
    sendCommandPGM(7);
    sendCommandPGM(8);
    sendCommandPGM(11);

  }
  else
  {
    sendCommandPGM(9);
    sendCommandPGM(10);
    sendCommandPGM(11);

  }
  if (forceNight == false)
  {
    sendCommandPGM(12);
    sendCommandPGM(13);
    sendCommandPGM(16);

  }
  else
  {
    sendCommandPGM(14);
    sendCommandPGM(15);
    sendCommandPGM(16);

  }

}



void updateInfo() {
  if (nxtemperatureAqua != temperatureAqua)
  {
    if (temperatureAqua != TEMP_ERROR)
    {

      char str_temp[3];
      dtostrf(temperatureAqua, 4, 1, str_temp);
      sendCommandPGM (23, str_temp, xcelc, NULL);
      nxtemperatureAqua = temperatureAqua;
    }
    else
    {
      sendCommandPGM (23, xdash, NULL);
    }
  }

  if (nxtemperatureFans != temperatureFans)
  {

    if (temperatureFans != TEMP_ERROR) {

      char str_temp[3];
      dtostrf(temperatureFans, 4, 1, str_temp);
      sendCommandPGM (25, str_temp, xcelc, NULL);
      nxtemperatureFans = temperatureFans;
    }
    else
    {
      sendCommandPGM (25, xdash, NULL);
    }

  }


  for (int i = 0 ; i < PWMS; i++)
  {
    if (pwm_list[i].pwmLast != pwm_list[i].pwmNow)
    {

      if (pwm_list[i].pwmNow == 0 || pwm_list[i].pwmStatus == 0)
      {
        sendCommandPGM (17 + i, xoff, NULL);
      }
      else if (pwm_list[i].pwmNow == pwm_list[i].pwmMin && pwm_list[i].pwmKeepLight )
      {
        sendCommandPGM (17 + i, xnight, NULL);
      }
      else
      {
        byte percent =  map((int)pwm_list[i].pwmNow, 0, 255, 0, 100);
        char buf[3] = {0};
        itoa(percent, buf, 10);

        sendCommandPGM (17 + i, buf, xpercent);
      }




      pwm_list[i].pwmLast = pwm_list[i].pwmNow;
    }

  }






  if (nxwaterFansStatus != waterFansStatus)
  {
    if (ledFansStatus)
    {
      sendCommandPGM (24, xon, NULL);
    }
    else
    {
      sendCommandPGM (24, xoff, NULL);

    }
    nxwaterFansStatus = waterFansStatus;
  }
  if (nxledFansStatus != ledFansStatus)
  {
    if (ledFansStatus)
    {
      sendCommandPGM (26, xon, NULL);
    }
    else
    {
      sendCommandPGM (26, xoff, NULL);

    }
    nxledFansStatus = ledFansStatus;
  }



}


void timeDisplay(tmElements_t tm) {
  if (nxLastMinute != tm.Minute || nxLastHour != tm.Hour)
  {
    memset(buffer, 0, sizeof (buffer));
    strcpy_P(buffer, (PGM_P)pgm_read_word(&(nxStrings[0])));
    sprintf(buffer + strlen(buffer), "%02u:%02u", tm.Hour, tm.Minute);
    strcpy_P(buffer + strlen(buffer), (PGM_P)pgm_read_word(&(nxStrings[1])));
    sendCommand(buffer);
    nxLastHour = tm.Hour;
    nxLastMinute =  tm.Minute;
  }
}

void nxsetTime ()
{
  sendCommandPGMInt(30, tm.Hour, true);
  sendCommandPGMInt(31, tm.Minute, true);
  sendCommandPGMInt(32, tm.Day, true);
  sendCommandPGMInt(33, tm.Month, true);
  sendCommandPGMInt(34, tm.Year, true);
}

void nxCore ()
{
  if (nxScreen == 0 )
  {
    if (currentMillis - previousNxInfo > NX_INFO_RESOLUTION)
    {
      previousNxInfo = currentMillis;
      timeDisplay(tm);
      updateInfo();
    }
  }
}


bool nexInit(void)
{


  bool ret1 = false;
  bool ret2 = false;

  Serial.begin(9600);
  sendCommand("");
  sendCommandPGM(27);
  ret2 = recvRetCommandFinished();
  sendCommandPGM(28);
  ret2 = recvRetCommandFinished();
}

void sendCommand(const char* cmd)
{
  while (Serial.available())
  {
    Serial.read();
  }
  Serial.print(cmd);
  Serial.write(0xFF);
  Serial.write(0xFF);
  Serial.write(0xFF);
}


bool recvRetCommandFinished()
{
  bool ret = false;
  uint8_t temp[4] = {0};


  if (sizeof(temp) != Serial.readBytes((char *)temp, sizeof(temp)))
  {
    ret = false;
  }

  if (temp[0] == NEX_RET_CMD_FINISHED
      && temp[1] == 0xFF
      && temp[2] == 0xFF
      && temp[3] == 0xFF
     )
  {
    ret = true;
  }
  return ret;
}

uint16_t recvRetString(char *buffer, uint16_t len, uint32_t timeout)
{
    uint16_t ret = 0;
    bool str_start_flag = false;
    uint8_t cnt_0xff = 0;
    String temp = String("");
    uint8_t c = 0;
    long start;    
    
    start = millis();
    while (millis() - start <= timeout)
    {
        while (Serial.available())
        {
            c = Serial.read();
            if (str_start_flag)
            {
                if (0xFF == c)
                {
                    cnt_0xff++;                    
                    if (cnt_0xff >= 3)
                    {
                        break;
                    }
                }
                else
                {
                    temp += (char)c;
                }
            }
            else if (NEX_RET_STRING_HEAD == c)
            {
                str_start_flag = true;
            }
        }
        
        if (cnt_0xff >= 3)
        {
            break;
        }
    }

    ret = temp.length();
    ret = ret > len ? len : ret;
    strncpy(buffer, temp.c_str(), ret);

    return ret;
}


int getInt ()
{
  char b[10]  = {};
  recvRetString (b, 10, 100);
  return atoi (b);
}




