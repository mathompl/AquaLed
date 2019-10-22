/*
Aqualed Bluetooth functions (c) T. Formanowski 2016-2017
https://github.com/mathompl/AquaLed
*/


#ifndef NO_BLUETOOTH

#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial bluetooth(A2, A1);

static char cmdOutputArray[64];

void setupBluetooth ()
{
        bluetooth.begin(9600);
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

        byte val[64];
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
                if (val[2] >= 0 && val[2] <= 1) pwmSettings[pwmNumber].enabled = val[2]; else return false;
                if (val[3] >= 0 && val[3] <= 23) pwmSettings[pwmNumber].onHour = val[3]; else return false;
                if (val[4] >= 0 && val[4] <= 59) pwmSettings[pwmNumber].onMinute = val[4]; else return false;
                if (val[6] >= 0 && val[6] <= 23) pwmSettings[pwmNumber].offHour = val[6]; else return false;
                if (val[7] >= 0 && val[7] <= 59) pwmSettings[pwmNumber].offMinute = val[7]; else return false;
                if (val[9] >= 0 && val[9] <= 255) pwmSettings[pwmNumber].valueNight = val[9]; else return false;
                if (val[10] >= 0 && val[10] <= 255) pwmSettings[pwmNumber].valueDay = val[10]; else return false;
                if (val[11] >= 0 && val[11] <= 255) pwmSettings[pwmNumber].sunriseLenght = val[11]; else return false;
                if (val[12] >= 0 && val[12] <= 255) pwmSettings[pwmNumber].sunsetLenght = val[12]; else return false;
                if (val[13] >= 0 && val[13] <= 1) pwmSettings[pwmNumber].isNightLight = val[13]; else return false;
                writeEEPROMPWMConfig (pwmNumber);
                return true;
        }

        /////////////////////////////////////////////////////////////////////////////////////////////
        /////                        Set Date                                                   /////
        /////////////////////////////////////////////////////////////////////////////////////////////
        static byte setHour, setMinute, setSecond, setYear, setMonth, setDay;

        if (val[0] == 40)
        {
                if (val[1] >= 0 && val[1] <= 23) setHour = val[1]; else return false;
                if (val[2] >= 0 && val[2] <= 59) setMinute = val[2]; else return false;
                if (val[3] >= 0 && val[3] <= 59) setSecond = val[3]; else return false;
                if (val[4] >= 15 && val[4] <= 99) setYear = val[4] + 2000; else return false;
                if (val[5] >= 1 && val[5] <= 99) setMonth = val[5]; else return false;
                if (val[6] >= 1 && val[6] <= 31) setDay = val[6]; else return false;
                RTC.adjust(DateTime(setYear, setMonth, setDay, setHour, setMinute,0));
                readTime ();
                adjustDST ();
                return true;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        /////                       Get Settings                                              /////
        ///////////////////////////////////////////////////////////////////////////////////////////
        if (val[0] == 66)
        {
                int eAddress = 0;
                // Serial.write("*66,");
                bluetooth.write( "*66,");

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

                bluetooth.print( F("*83" ));
                bluetooth.print( F("," ));
                bluetooth.print( RTC.now().hour() );
                bluetooth.print( F("," ) );
                bluetooth.print( RTC.now().minute() );
                bluetooth.print( F("," ));
                bluetooth.print( RTC.now().second() );
                bluetooth.print( F("," ));
                bluetooth.print( tmYearToCalendar( RTC.now().year() ) );
                bluetooth.print( F(","));
                bluetooth.print( RTC.now().month() );
                bluetooth.print( F(","));
                bluetooth.print( RTC.now().day() );
                bluetooth.println( );

        }

        if (val[0] == 80)
        {
                bluetooth.println( F ("Aqualed 1.0 - BT" ));
                //return true;
        }
        return true;
}

void bluetoothServe ()
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
                if (commandAnalysis(cmdOutputArray))
                        eEpromRead();
                else
                        bluetooth.print(F("666,Bledne dane\n"));


        }

        cmd = "";
}
#endif
