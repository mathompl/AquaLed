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

// kompatybilnosc z AQMA
static int getEEPROMAddr( byte n ) {
  if (n == 0) return 280;
  if (n == 1) return 300;
  if (n == 2) return 330;
  if (n == 3) return 370;
  if (n == 4) return 390;
  if (n == 5) return 410;

  // nie aqma
  if (n == 6) return 450;
  if (n == 7) return 470;

  return 0;
}

static byte translateAqmaNumber ( byte n ) {
  if (n == 31) return 0;
  if (n == 32) return 1;
  if (n == 33) return 2;
  if (n == 90) return 3;
  if (n == 91) return 4;
  if (n == 92) return 5;
  return 255;
}


void writeEEPROMPWMConfig (byte pwmNumber)
{
  int startAddr = getEEPROMAddr( pwmNumber );
  EEPROM.write( startAddr + 1,  pwm_list[pwmNumber].pwmPin );
  EEPROM.write( startAddr + 2,  pwm_list[pwmNumber].pwmStatus );
  EEPROM.write( startAddr + 3,  pwm_list[pwmNumber].pwmHOn );
  EEPROM.write( startAddr + 4,  pwm_list[pwmNumber].pwmMOn );
  EEPROM.write( startAddr + 5,  pwm_list[pwmNumber].pwmSOn );
  EEPROM.write( startAddr + 6,  pwm_list[pwmNumber].pwmHOff );
  EEPROM.write( startAddr + 7,  pwm_list[pwmNumber].pwmMOff );
  EEPROM.write( startAddr + 8,  pwm_list[pwmNumber].pwmSOff );
  EEPROM.write( startAddr + 9,  pwm_list[pwmNumber].pwmMin );
  EEPROM.write( startAddr + 10,  pwm_list[pwmNumber].pwmMax );
  EEPROM.write( startAddr + 11,  pwm_list[pwmNumber].pwmSr );
  EEPROM.write( startAddr + 12,  pwm_list[pwmNumber].pwmSs );
  EEPROM.write( startAddr + 13,  pwm_list[pwmNumber].pwmKeepLight );
  EEPROM.write( startAddr + 15,  pwm_list[pwmNumber].pwmAmbient );
}

void writeEEPROMPWMState (byte pwmNumber)
{
  int startAddr = getEEPROMAddr( pwmNumber );
  EEPROM.write( startAddr + 14,  pwm_list[pwmNumber].pwmTest );
  EEPROM.write( startAddr + 16,  (byte) pwm_list[pwmNumber].pwmNow );
}

static boolean isFirstRun ()
{
  
  if (EEPROM.read( 100 ) != 255) return true;
  else return false;
}

boolean writeEEPROMDefaults ()
{
  if (isFirstRun())
  {
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    }
    //defaults
    SETTINGS.forceNight = 0;
    SETTINGS.forceAmbient = 0;
    SETTINGS.forceOFF = 0;
    SETTINGS.max_led_temp = 35;
    SETTINGS.max_sump_temp = 35;
    SETTINGS.max_water_temp = 26;
    SETTINGS.pwmDimmingTime = 30;
    SETTINGS.screenSaverTime = 30;
    SETTINGS.softDimming = 0;
    SETTINGS.dst = 0;
    for (int i = 0 ; i < 8; i++)
    {
      SETTINGS.waterSensorAddress[i] = 0;
    }
    for (int i = 0 ; i < 8; i++)
    {
      SETTINGS.ledSensorAddress[i] = 0;
    }
    for (int i = 0 ; i < 8; i++)
    {
      SETTINGS.sumpSensorAddress[i] = 0;
    }

    for (int i = 0 ; i < PWMS; i++ )
    {
      pwm_list[i].pwmStatus = 0;
      pwm_list[i].pwmHOn = 0;
      pwm_list[i].pwmMOn = 0;
      pwm_list[i].pwmSOn = 0;
      pwm_list[i].pwmHOff = 0;
      pwm_list[i].pwmMOff = 0;
      pwm_list[i].pwmSOff = 0;
      pwm_list[i].pwmMin = 0;
      pwm_list[i].pwmMax = 0;
      pwm_list[i].pwmSr = 0;
      pwm_list[i].pwmSs = 0;
      pwm_list[i].pwmKeepLight = 0;
      pwm_list[i].pwmInvert = 0;
      pwm_list[i].pwmNow = 0;
      pwm_list[i].pwmLast = 0;
      pwm_list[i].pwmGoal = 0;
      pwm_list[i].pwmSaved = 0;
      pwm_list[i].pwmTest = 0;
      pwm_list[i].isSunrise = 0;
      pwm_list[i].isSunset = 0;
      pwm_list[i].pwmAmbient = 0;

      writeEEPROMPWMConfig (i);
      writeEEPROMPWMState (i);
    }
    writeEEPROMSettings ();
    EEPROM.write( 100, 255 );

  }
}

boolean writeEEPROMSettings ()
{
  EEPROM.write( 101, SETTINGS.forceNight);
  EEPROM.write( 102, SETTINGS.forceAmbient);
  EEPROM.write( 103, SETTINGS.forceOFF);
  EEPROM.write( 104, SETTINGS.max_led_temp);
  EEPROM.write( 105, SETTINGS.max_water_temp);
  EEPROM.write( 106, SETTINGS.pwmDimmingTime);
  EEPROM.write( 107, SETTINGS.max_sump_temp);
  EEPROM.write( 108, SETTINGS.screenSaverTime);
  EEPROM.write( 109, SETTINGS.softDimming);
  EEPROM.write( 110, SETTINGS.dst);


  // 24 bajty adresy sensoow
  byte startAddr = 120;
  for (int i = 0 ; i < 8; i++)
  {
    EEPROM.write( startAddr + i, SETTINGS.ledSensorAddress[i]);
  }
  startAddr = 130;
  for (int i = 0 ; i < 8; i++)
  {
    EEPROM.write( startAddr + i, SETTINGS.sumpSensorAddress[i]);
  }
  startAddr = 140;
  for (int i = 0 ; i < 8; i++)
  {
    EEPROM.write( startAddr + i, SETTINGS.waterSensorAddress[i]);
  }

}

boolean readEEPROMSettings ()
{
  SETTINGS.forceNight = EEPROM.read( 101 );
  SETTINGS.forceAmbient = EEPROM.read( 102);
  SETTINGS.forceOFF = EEPROM.read( 103);
  SETTINGS.max_led_temp = EEPROM.read( 104);
  SETTINGS.max_water_temp = EEPROM.read( 105);
  SETTINGS.pwmDimmingTime = EEPROM.read( 106);
  SETTINGS.max_sump_temp = EEPROM.read( 107);
  SETTINGS.screenSaverTime = EEPROM.read( 108);
  SETTINGS.softDimming = EEPROM.read( 109);
  SETTINGS.dst = EEPROM.read( 110);

  // 24 bajty adresy sensoow
  byte startAddr = 120;
  for (int i = 0 ; i < 8; i++)
  {
    SETTINGS.ledSensorAddress[i] = EEPROM.read( startAddr + i);
  }
  startAddr = 130;
  for (int i = 0 ; i < 8; i++)
  {
    SETTINGS.sumpSensorAddress[i] = EEPROM.read( startAddr + i);
  }
  startAddr = 140;
  for (int i = 0 ; i < 8; i++)
  {
    SETTINGS.waterSensorAddress[i] = EEPROM.read( startAddr + i);
  }
}



void eEpromRead( ) {
  for (int i = 0; i < PWMS; i++)
  {
    int startAddr = getEEPROMAddr( i );
    pwm_list[i].pwmStatus = EEPROM.read( startAddr + 2 );
    pwm_list[i].pwmHOn = EEPROM.read( startAddr + 3 );
    pwm_list[i].pwmMOn = EEPROM.read( startAddr + 4 );
    pwm_list[i].pwmSOn = EEPROM.read( startAddr + 5 );
    pwm_list[i].pwmHOff = EEPROM.read( startAddr + 6 );
    pwm_list[i].pwmMOff = EEPROM.read( startAddr + 7 );
    pwm_list[i].pwmSOff = EEPROM.read( startAddr + 8 );
    pwm_list[i].pwmMin = EEPROM.read( startAddr + 9 );
    pwm_list[i].pwmMax = EEPROM.read( startAddr + 10 );
    pwm_list[i].pwmSr = EEPROM.read( startAddr + 11 );
    pwm_list[i].pwmSs = EEPROM.read( startAddr + 12 );
    pwm_list[i].pwmKeepLight = EEPROM.read( startAddr + 13 );
    pwm_list[i].pwmTest = EEPROM.read( startAddr + 14 );
    pwm_list[i].pwmAmbient = EEPROM.read( startAddr + 15 );
    pwm_list[i].pwmSaved = EEPROM.read( startAddr + 16);

  }
  readEEPROMSettings ();

}
