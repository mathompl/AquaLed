void sendCommand(const char* cmd)
{  
    nexSerial.print(cmd);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
}

bool nexInit(void)
{
    bool ret1 = false;
    bool ret2 = false;

    Serial.begin(9600);
    sendCommand("");
    sendCommand("bkcmd=1");
    ret1 = recvRetCommandFinished();
    sendCommand("page 0");
    ret2 = recvRetCommandFinished();
    return ret1 && ret2;
}

bool recvRetCommandFinished(uint32_t timeout)
{    
    bool ret = false;
    uint8_t temp[4] = {0};
    
    Serial.setTimeout(timeout);
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

    if (ret) 
    {
//        dbSerialPrintln("recvRetCommandFinished ok");
    }
    else
    {
//        dbSerialPrintln("recvRetCommandFinished err");
    }

    return ret;
}

/*
void nexLoop(NexTouch *nex_listen_list[])
{
    static uint8_t __buffer[10];
    
    uint16_t i;
    uint8_t c;  
    
    while (nexSerial.available() > 0)
    {   
        delay(10);
        c = nexSerial.read();
        
        if (NEX_RET_EVENT_TOUCH_HEAD == c)
        {
            if (nexSerial.available() >= 6)
            {
                __buffer[0] = c;  
                for (i = 1; i < 7; i++)
                {
                    __buffer[i] = nexSerial.read();
                }
                __buffer[i] = 0x00;
                
                if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6])
                {
                    NexTouch::iterate(nex_listen_list, __buffer[1], __buffer[2], (int32_t)__buffer[3]);
                }
                
            }
        }
    }
}

*/
static void timeSaveButtonCallback (void *ptr)
{
}
static void timeCancelButtonCallback (void *ptr)
{
  sendCommand ("config"+nxShow);
  nxScreen = 1;
}


static void configOtherButtonCallback (void *ptr)
{
}
static void configTimeButtonCallback (void *ptr)
{
  sendCommand ("time"+nxShow);
   nxScreen = 2;
}

static void configLED1ButtonCallback (void *ptr)
{
}
static void configLED2ButtonCallback (void *ptr)
{
}
static void configLED3ButtonCallback (void *ptr)
{
}
static void configLED4ButtonCallback (void *ptr)
{
}
static void configLED5ButtonCallback (void *ptr)
{
}
static void configLED6ButtonCallback (void *ptr)
{
}



static void configBackButtonCallback (void *ptr)
{
  
    sendCommand ("home"+nxShow);
 nxScreen = 0; 
  toggleButtons();
}




void nexTouch()
{
     nexLoop(nex_Listen_List);
     
}




char buffer[20] = {0};

static void offButtonCallback(void *ptr)
{
  if (forceOFF == true) forceOFF = false;  else  forceOFF = true;  
  toggleButtons();
}


static void ambientButtonCallback(void *ptr)
{
  if (forceOFF == true || forceNight == true) return;
  if (forceAmbient == true) forceAmbient = false;  else  forceAmbient = true;  
  toggleButtons();
}

static void nightButtonCallback(void *ptr)
{
  if (forceAmbient  == true || forceOFF  == true) return;
  if (forceNight == true) forceNight = false;  else  forceNight = true;  
  toggleButtons();
}

static void configButtonCallback(void *ptr)
{
  config.show ();  
   nxScreen = 1;
  
}

static void toggleButtons()
{
  if (forceOFF == false)
  {  
  sendCommand("bo.pic=3"); //set "b0" image to 2
  sendCommand("bo.pic2=3"); //set "b0" image to 2
  sendCommand("ref bo"); //refresh

}
else
{
  sendCommand("bo.pic=2"); //set "b0" image to 2
  sendCommand("bo.pic2=2"); //set "b0" image to 2
  sendCommand("ref bo"); //refresh
}

if (forceAmbient == false)
{  
  sendCommand("ba.pic=6"); //set "b0" image to 2
  sendCommand("ba.pic2=6"); //set "b0" image to 2
  sendCommand("ref ba"); //refresh
}
else
{
  sendCommand("ba.pic=7"); //set "b0" image to 2
  sendCommand("ba.pic2=7"); //set "b0" image to 2
  sendCommand("ref ba"); //refresh
}
  if (forceNight == false)
{  
  sendCommand("bn.pic=4"); //set "b0" image to 2
  sendCommand("bn.pic2=4"); //set "b0" image to 2
  sendCommand("ref bn"); //refresh

}
else
{
  sendCommand("bn.pic=5"); //set "b0" image to 2
  sendCommand("bn.pic2=5"); //set "b0" image to 2
  sendCommand("ref bn"); //refresh
}


}


static void updateLedStatus (byte number)
{

    memset(buffer, 0, sizeof (buffer));
    if (pwm_list[number].pwmNow == 0 || pwm_list[number].pwmStatus ==0)
    {
      strcat(buffer, "OFF");    
    }
    else
    if (pwm_list[number].pwmNow == pwm_list[number].pwmMin && pwm_list[number].pwmKeepLight )
    {
            strcat(buffer, "NIGHT");    
    }
    else
    {
      byte percent =  map((int)pwm_list[number].pwmNow, 0, 255, 0, 100);
      itoa(percent, buffer, 10);
      strcat(buffer, "%");    
    }
    ledTexts[number].setText(buffer);
}

static void updateFanStatus (NexText nxText, boolean val)
{
    memset(buffer, 0, sizeof (buffer));
    if (val)
    {
      strcat(buffer, "ON");    
    }
    else
    {
      strcat(buffer, "OFF");        
    }
    nxText.setText(buffer);
}

void updateInfo() {
      if (nxtemperatureAqua!=temperatureAqua)
      {
       if (temperatureAqua != TEMP_ERROR) 
       {      
        memset(buffer, 0, sizeof (buffer));        
        char tempAqua[10];
        char str_temp[6];
        dtostrf(temperatureAqua, 4, 1, str_temp);
        strcat(buffer, str_temp);          
        strcat(buffer, "'C");          
        nxTempWater.setText(buffer);
        nxtemperatureAqua = temperatureAqua;
      }         
    else
    {
        memset(buffer, 0, sizeof (buffer));
        strcat(buffer, "-");                
        nxTempWater.setText(buffer);      
    
    }
      }
      
     if (nxtemperatureFans!=temperatureFans)
     {  

       if (temperatureFans != TEMP_ERROR) {
        memset(buffer, 0, sizeof (buffer));
        char str_temp[6];
        char tempFans[10];
        dtostrf(temperatureFans, 4, 1, str_temp);
        strcat(buffer, str_temp);          
        strcat(buffer, "'C");          
        nxTempLed.setText(buffer);
        nxtemperatureFans = temperatureFans;
    }
    else
    {
        memset(buffer, 0, sizeof (buffer));
        strcat(buffer, "-");                
        nxTempLed.setText(buffer);
    }
    
}    
   
  
    for (int i =0 ; i < PWMS;i++)
    {
       if (pwm_list[i].pwmLast!=pwm_list[i].pwmNow) 
        {
          updateLedStatus (i); 
          pwm_list[i].pwmLast=pwm_list[i].pwmNow;
        }
    
    }
   

    
    
                        

    if (nxwaterFansStatus!=waterFansStatus) {updateFanStatus (nxWaterFans, waterFansStatus);nxwaterFansStatus = waterFansStatus;}
    if (nxledFansStatus!=ledFansStatus) {updateFanStatus (nxLedFans, ledFansStatus);nxledFansStatus = ledFansStatus;}
    
    
    
}

void timeDisplay(tmElements_t tm) {
  if (nxLastMinute!=tm.Minute || nxLastHour != tm.Hour)
  {
     memset(buffer, 0, sizeof (buffer));
     sprintf(buffer, "%02u:%02u", tm.Hour, tm.Minute);
     nxHour.setText(buffer);
     nxLastHour = tm.Hour;
     nxLastMinute =  tm.Minute;
  }


}


void nxCore ()
{
    if (nxScreen ==0 )
    {
if (currentMillis - previousNxInfo > NX_INFO_RESOLUTION) 
    {
      previousNxInfo = currentMillis;
      timeDisplay(tm);
      updateInfo();
    }
    }
    nexTouch ();

}













