
void setupLedPins ()
{
  // assign pins
  pwm_list[0].pwmPin = PWM1_PIN;
  pwm_list[1].pwmPin = PWM2_PIN;
  pwm_list[2].pwmPin = PWM3_PIN;
  pwm_list[3].pwmPin = PWM4_PIN;
  pwm_list[4].pwmPin = PWM5_PIN;
  pwm_list[5].pwmPin = PWM6_PIN;

  // setup pins
  for (int i = 0 ; i < PWMS; i++)
  {
    pinMode(pwm_list[i].pwmPin, OUTPUT);
    digitalWrite(pwm_list[i].pwmPin, OFF);
    pwm_list[i].pwmNow = 0;


  }

  //TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
  TCCR1A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
  TCCR2A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);

}


boolean pwmStep (byte i, long silkyTime)
{
  boolean dimming = false;
  double pwmNow = pwm_list[i].pwmNow;
  int pwmGoal = pwm_list[i].pwmGoal;


  // mamy co chcemy, precz
  if (pwmNow == pwmGoal) return dimming;


  double step;
  //   if (pwmGoal>pwmNow) step = (double)( (double)pwmGoal/(double)( silkyTime/PWM_RESOLUTION) );
  //   else step = (double) ((double)pwmNow/( (double)silkyTime/PWM_RESOLUTION));
  step = (double) (double)255 / (double)(silkyTime / PWM_RESOLUTION);

  byte stepsLeft = (pwmGoal - pwmNow) / step;
  if (stepsLeft < 0) stepsLeft *= -1;
  long millisLeft = stepsLeft * PWM_RESOLUTION;

  //   long estTimeEnd = currentMillis+millisLeft;

  if (pwmGoal > pwmNow)
  {
    pwmNow = pwmNow + step;
    if (pwmNow > pwmGoal) pwmNow = pwmGoal;


  }
  else
  {
    pwmNow = pwmNow - step;
    if (pwmNow < 0 || pwmNow < 0.01) pwmNow = 0;
  }
  pwm_list[i].pwmNow = pwmNow;
}




static byte pwm( byte i )
{
  long ssMillis;
  long srMillis;
  boolean state = false;
  boolean dimming = false;
  long pwmOn = (long(pwm_list[i].pwmHOn) * 3600) + (long(pwm_list[i].pwmMOn) * 60) + long(pwm_list[i].pwmSOn);
  long pwmOff = (long(pwm_list[i].pwmHOff) * 3600) + (long(pwm_list[i].pwmMOff) * 60) + long(pwm_list[i].pwmSOff);

  if (pwmOn < pwmOff) if (currentTimeSec >= pwmOn && currentTimeSec < pwmOff)    state = true;

  if (pwmOn > pwmOff)
  {
    if (currentTimeSec >= pwmOn && currentTimeSec <= 86400) state = true;
    if (currentTimeSec >= 0 && currentTimeSec < pwmOff) state = true;

  }
  if (pwm_list[i].pwmStatus == 0 || forceOFF == true)
  {
    pwm_list[i].pwmGoal = 0;
    dimming = pwmStep (i, PWM_DIMMING_TIME);
  }
  else
    // night
    if (forceNight)
    {
      if (pwm_list[i].pwmKeepLight) pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
      else
        pwm_list[i].pwmGoal = 0;
      dimming = pwmStep (i, PWM_DIMMING_TIME);
    }
    else
      // ambient
      if (forceAmbient == true)
      {
        pwm_list[i].pwmGoal = AMBIENT_VALUE;
        dimming = pwmStep (i, PWM_DIMMING_TIME);
      }
      else if (!state && pwm_list[i].pwmKeepLight)
      {
        pwm_list[i].pwmGoal = pwm_list[i].pwmMin;
        dimming = pwmStep (i, PWM_DIMMING_TIME);
      }

      else
        if (!state && !pwm_list[i].pwmKeepLight)
        {
          pwm_list[i].pwmGoal = 0;
          dimming = pwmStep (i, PWM_DIMMING_TIME);
        }
        else
          //sunset
          if ( getSunsetMillis(i, ssMillis) > 0)
          {
            pwm_list[i].pwmGoal = 0;
            dimming = pwmStep (i, ssMillis);
          }
          else
            //sunrise
            if ( getSunriseMillis(i, srMillis) > 0)
            {
              pwm_list[i].pwmGoal = pwm_list[i].pwmMax;
              dimming = pwmStep (i, srMillis);
            }

            else if (state)
            {
              pwm_list[i].pwmGoal = pwm_list[i].pwmMax;
              dimming = pwmStep (i, PWM_DIMMING_TIME);
            }
  int val = (int) pwm_list[i].pwmNow;

  // table
  if (dimming)
  {


  }

  analogWrite( pwm_list[i].pwmPin, (byte) val);

}



long getSunsetMillis (byte i, long &m)
{

  // w sekundach
  int stopTime = pwm_list[i].pwmHOff * 60 * 60 + pwm_list[i].pwmMOff * 60 - pwm_list[i].pwmSOff ;
  int currTime = tm.Hour * 60 * 60 + tm.Minute * 60 + tm.Second;
  int startTime = stopTime - pwm_list[i].pwmSs * 60;
  if (stopTime < startTime) stopTime += 86400;

  if (currTime >= startTime && currTime <= stopTime)
  {
    m = (long) (stopTime - currTime) * 1000;
    Serial.println (m);
    return m;

  }
  m = 0;
  return m;

}

long getSunriseMillis (byte i, long &m)
{

  // w sekundach
  int startTime = pwm_list[i].pwmHOn * 60 * 60 + pwm_list[i].pwmMOn * 60 + pwm_list[i].pwmSOn;
  int currTime = tm.Hour * 60 * 60 + tm.Minute * 60 + tm.Second;
  int stopTime = startTime + pwm_list[i].pwmSr * 60;
  if (stopTime < startTime) stopTime += 86400;

  if (currTime >= startTime && currTime <= stopTime)
  {
    m = (long) (stopTime - currTime) * 1000;
    return m;

  }
  m = 0;
  return m;
}


int getEEPROMAddr( byte n ) {
  if (n == 0) return 280;
  if (n == 1) return 300;
  if (n == 2) return 330;
  if (n == 3) return 370;
  if (n == 4) return 390;
  if (n == 5) return 410;
  return 0;
}

byte translateAqmaNumber ( byte n ) {
  if (n == 31) return 0;
  if (n == 32) return 1;
  if (n == 33) return 2;
  if (n == 90) return 3;
  if (n == 91) return 4;
  if (n == 92) return 5;
  return 255;
}


boolean commandAnalysis( char cmdOutputArray[64] ) {

  unsigned int val[64];
  char *cmdVal;
  int i = 0;

  cmdVal = strtok( cmdOutputArray, "," );

  while (cmdVal)
  {
    val[i++] = atoi( cmdVal );
    cmdVal = strtok( NULL, "," );
  }




  byte pwmNumber = translateAqmaNumber (val[0]);
  // konfig PWM

  if (pwmNumber < 255)
  {


    byte ePwm1Pin, ePwm1Status, ePwm1HOn, ePwm1MOn, ePwm1SOn, ePwm1HOff, ePwm1MOff, ePwm1SOff, ePwm1Min, ePwm1Max, ePwm1Sr, ePwm1Ss, ePwm1KeepLight;

    if (val[1] >= 0 && val[1] <= 99)        {
      ePwm1Pin = val[1];         // ePwm1Pin -       1
    } else        {
      return false;
    }
    if (val[2] >= 0 && val[2] <= 1)        {
      ePwm1Status = val[2];         // ePwm1Status -    2
    } else        {
      return false;
    }
    if (val[3] >= 0 && val[3] <= 23)        {
      ePwm1HOn = val[3];         // ePwm1HOn -       3
    } else        {
      return false;
    }
    if (val[4] >= 0 && val[4] <= 59)        {
      ePwm1MOn = val[4];         // ePwm1MOn -       4
    } else        {
      return false;
    }
    if (val[5] >= 0 && val[5] <= 59)        {
      ePwm1SOn = val[5];         // ePwm1SOn -       5
    } else        {
      return false;
    }
    if (val[6] >= 0 && val[6] <= 23)        {
      ePwm1HOff = val[6];         // ePwm1HOff -      6
    } else        {
      return false;
    }
    if (val[7] >= 0 && val[7] <= 59)        {
      ePwm1MOff = val[7];         // ePwm1MOff -      7
    } else        {
      return false;
    }
    if (val[8] >= 0 && val[8] <= 59)       {
      ePwm1SOff = val[8];         // ePwm1SOff -      8
    } else        {
      return false;
    }
    if (val[9] >= 0 && val[9] <= 255)        {
      ePwm1Min = val[9];         // ePwm1Min -       9
    } else        {
      return false;
    }
    if (val[10] >= 0 && val[10] <= 255)        {
      ePwm1Max = val[10];         // ePwm1Max -       10
    } else        {
      return false;
    }
    if (val[11] >= 0 && val[11] <= 255)        {
      ePwm1Sr = val[11];         // ePwm1Sr -        11
    } else        {
      return false;
    }
    if (val[12] >= 0 && val[12] <= 255)        {
      ePwm1Ss = val[12];         // ePwm1Ss -        12
    } else        {
      return false;
    }
    if (val[13] >= 0 && val[13] <= 1)        {
      ePwm1KeepLight = val[13];         // ePwm1KeepLight - 13
    } else        {
      return false;
    }

    int startAddr = getEEPROMAddr( pwmNumber );


    EEPROM.write( startAddr + 1, ePwm1Pin );
    EEPROM.write( startAddr + 2, ePwm1Status );
    EEPROM.write( startAddr + 3, ePwm1HOn );
    EEPROM.write( startAddr + 4, ePwm1MOn );
    EEPROM.write( startAddr + 5, ePwm1SOn );
    EEPROM.write( startAddr + 6, ePwm1HOff );
    EEPROM.write( startAddr + 7, ePwm1MOff );
    EEPROM.write( startAddr + 8, ePwm1SOff );
    EEPROM.write( startAddr + 9, ePwm1Min );
    EEPROM.write( startAddr + 10, ePwm1Max );
    EEPROM.write( startAddr + 11, ePwm1Sr );
    EEPROM.write( startAddr + 12, ePwm1Ss );
    EEPROM.write( startAddr + 13, ePwm1KeepLight );
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  /////                        Set Date                                                   /////
  /////////////////////////////////////////////////////////////////////////////////////////////
  static byte setHour, setMinute, setSecond, setYear, setMonth, setDay;

  if (val[0] == 40)
  {
    if (val[1] >= 0 && val[1] <= 23)        {
      setHour = val[1];         // setHour -   1
    } else        {
      return false;
    }
    if (val[2] >= 0 && val[2] <= 59)        {
      setMinute = val[2];         // setMinute - 2
    } else        {
      return false;
    }
    if (val[3] >= 0 && val[3] <= 59)        {
      setSecond = val[3];         // setSecond - 3
    } else        {
      return false;
    }
    if (val[4] >= 15 && val[4] <= 99)        {
      setYear = val[4] + 2000;         // setYear -   4
    } else        {
      return false;
    }
    if (val[5] >= 1 && val[5] <= 99)        {
      setMonth = val[5];         // setMonth -  5
    } else        {
      return false;
    }
    if (val[6] >= 1 && val[6] <= 31)        {
      setDay = val[6];         // setDay -    5
    } else        {
      return false;
    }

    setTime( setHour, setMinute, setSecond, setDay, setMonth, setYear ); // godz, min, sek, dzien, miesiac, rok

    RTC.set( now( ) );
    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////////////////
  /////                       Get Settings                                              /////
  ///////////////////////////////////////////////////////////////////////////////////////////
  if (val[0] == 66)
  {
    int eAddress = 0;
    // Serial.write("*66,");
    bluetooth.write( "*66," );

    while (eAddress <= 512)
    {
      //   Serial.print(EEPROM.read(eAddress));
      bluetooth.print( EEPROM.read( eAddress ) );
      // Serial.write(",");
      bluetooth.write( "," );
      eAddress++;
    }
    //   Serial.write("#\n");
    bluetooth.write( "#\n" );
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  /////                       Get Date                                                  /////
  ///////////////////////////////////////////////////////////////////////////////////////////
  if (val[0] == 83)
  {

    bluetooth.print( "*83" );
    bluetooth.write( "," );
    bluetooth.print( tm.Hour );
    bluetooth.write( "," );
    bluetooth.print( tm.Minute );
    bluetooth.write( "," );
    bluetooth.print( tm.Second );
    bluetooth.write( "," );
    bluetooth.print( tmYearToCalendar( tm.Year ) );
    bluetooth.write( "," );
    bluetooth.print( tm.Month );
    bluetooth.write( "," );
    bluetooth.print( tm.Day );
    bluetooth.println( );

  }

  if (val[0] == 80)
  {
    //          Serial.println("AQma LED Control, 2.0.1 - BT");
    bluetooth.println( "AQma LED Control, 2.0.1 - BT" );
    //return true;
  }


}




///////////////////////////////////////////////////////////////////////////
/////                       eEpromRead                                /////
///////////////////////////////////////////////////////////////////////////

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
  }
}





void bluetoothCore ()
{
  char cmdChar = 0;
  String cmd = "";
  while (bluetooth.available() > 0) {
    cmdChar = bluetooth.read();
    delay(5);
    cmd.concat(cmdChar);
  }

  if (cmd != "") {

    cmd.toCharArray(cmdOutputArray, 64);
    //    Serial.println (cmdOutputArray);
    if (commandAnalysis(cmdOutputArray)) {
      eEpromRead();
    }

    else {
      //             Serial.print("666,Bledne dane\n");
      bluetooth.print("666,Bledne dane\n");
    }

  }

  cmd = "";
}

void ledCore ()
{

  if (currentMillis - previousPwmResolution > PWM_RESOLUTION)
  {
    previousPwmResolution = currentMillis;
    for (int i = 0 ; i < PWMS ; i++)
    {
      pwm(i);

    }
  }

}





















