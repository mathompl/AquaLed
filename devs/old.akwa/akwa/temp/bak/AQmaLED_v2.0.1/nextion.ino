#include "NexButton.h"
#include "NexText.h"
#include "NexPage.h"
#include "NexPicture.h"
#include "NexTouch.h"
//#define at_serial Serial;


NexText nxHour = NexText(0, 5, "hour");

NexText nxTempWater = NexText(0, 6, "wt");
NexText nxTempLed = NexText(0, 8, "lt");

NexText nxled1 = NexText(0, 10, "l1");
NexText nxled2 = NexText(0, 12, "l2");
NexText nxled3 = NexText(0, 14, "l3");
NexText nxled4 = NexText(0, 16, "l4");
NexText nxled5 = NexText(0, 18, "l5");
NexText nxled6 = NexText(0, 20, "l6");

NexText nxWaterFans = NexText(0, 26, "wf");
NexText nxLedFans = NexText(0, 24, "lf");

NexButton offButton = NexButton(0, 23, "bo");
NexButton ambientButton = NexButton(0, 26, "ba");
NexButton nightButton = NexButton(0, 24, "bn");
NexHotspot configButton = NexHotspot(0, 25, "m1");

NexPage home = NexPage(0, 0, "home");
NexPage config = NexPage(0, 4, "config");

NexButton configBackButton = NexButton(4, 11, "b8");

NexTouch *nex_Listen_List[] = 
{
    &offButton,
    &ambientButton,
    &nightButton,
    &configButton,

    &config,
    &configBackButton,
    
    NULL
};

void setupPopups ()
{
      offButton.attachPop (offButtonCallback);
      ambientButton.attachPop (ambientButtonCallback);
      nightButton.attachPop (nightButtonCallback);
      configButton.attachPop (configButtonCallback);
 
      configBackButton.attachPop (configBackButtonCallback);   
      
      
}

static void configBackButtonCallback (void *ptr)
{
  
  home.show ();  
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
  sendCommand("bo.pic=8"); //set "b0" image to 2
  sendCommand("bo.pic2=8"); //set "b0" image to 2
  sendCommand("ref bo"); //refresh

}
else
{
  sendCommand("bo.pic=9"); //set "b0" image to 2
  sendCommand("bo.pic2=9"); //set "b0" image to 2
  sendCommand("ref bo"); //refresh
}

if (forceAmbient == false)
{  
  sendCommand("ba.pic=12"); //set "b0" image to 2
  sendCommand("ba.pic2=12"); //set "b0" image to 2
  sendCommand("ref ba"); //refresh
}
else
{
  sendCommand("ba.pic=13"); //set "b0" image to 2
  sendCommand("ba.pic2=13"); //set "b0" image to 2
  sendCommand("ref ba"); //refresh
}
  if (forceNight == false)
{  
  sendCommand("bn.pic=10"); //set "b0" image to 2
  sendCommand("bn.pic2=10"); //set "b0" image to 2
  sendCommand("ref bn"); //refresh

}
else
{
  sendCommand("bn.pic=11"); //set "b0" image to 2
  sendCommand("bn.pic2=11"); //set "b0" image to 2
  sendCommand("ref bn"); //refresh
}


}


static void updateLedStatus (NexText nxText, byte ledStatus, byte pwmStatus, byte pwmMin, byte keeplight)
{

  memset(buffer, 0, sizeof (buffer));
    if (ledStatus == 0 || pwmStatus ==0)
    {
      strcat(buffer, "OFF");    
    }
    else
    if (ledStatus == pwmMin && keeplight )
    {
            strcat(buffer, "NIGHT");    
    }
    else
    {
      byte percent =  map(ledStatus, 0, 255, 0, 100);
      itoa(percent, buffer, 10);
      strcat(buffer, "%");    
    }
    nxText.setText(buffer);
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
        
    }
    else
    {
        memset(buffer, 0, sizeof (buffer));
        strcat(buffer, "-");                
        nxTempLed.setText(buffer);
    }
    
}    
  nxtemperatureAqua = temperatureAqua;
        nxtemperatureFans = temperatureFans;
    
    if (nxled1statuspercent!=led1status) {updateLedStatus ( nxled1,  led1status,  pwm1Status, pwm1Min, pwm1KeepLight); nxled1statuspercent  = led1status;}
    if (nxled2statuspercent!=led2status) {updateLedStatus ( nxled2,  led2status,  pwm2Status, pwm2Min, pwm2KeepLight); nxled2statuspercent = led2status;}
    if (nxled3statuspercent!=led3status) {updateLedStatus ( nxled3,  led3status,  pwm3Status, pwm3Min, pwm3KeepLight);  nxled3statuspercent = led3status;}
    if (nxled4statuspercent!=led4status) {updateLedStatus ( nxled4,  led4status,  pwm4Status, pwm4Min, pwm4KeepLight);    nxled4statuspercent = led4status;}
    if (nxled5statuspercent!=led5status) {updateLedStatus ( nxled5,  led5status,  pwm5Status, pwm5Min, pwm5KeepLight);nxled5statuspercent = led5status;}
    if (nxled6statuspercent!=led6status) {updateLedStatus ( nxled6,  led6status,  pwm6Status, pwm6Min, pwm6KeepLight);nxled6statuspercent = led6status;}
   

    
    
                        

    if (nxwaterFansStatus!=waterFansStatus) {updateFanStatus (nxWaterFans, waterFansStatus);nxwaterFansStatus = waterFansStatus;}
    if (nxledFansStatus!=ledFansStatus) {updateFanStatus (nxLedFans, ledFansStatus);nxledFansStatus = ledFansStatus;}
    
    
    
}

void timeDisplay(tmElements_t tm) {
  if (nxLastMinute!=tm.Minute || nxLastHour != tm.Hour)
  {
  memset(buffer, 0, sizeof (buffer));
    sprintf(buffer, "%02u:%02u", tm.Hour, tm.Minute);
    nxHour.setText(buffer);
  }
  nxLastHour = tm.Hour;
  nxLastMinute =  tm.Minute;

}













