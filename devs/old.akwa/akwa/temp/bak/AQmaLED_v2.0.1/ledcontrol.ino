
////////////////////////////////////////////////////////////////////////////////////////
/////                               PWM Function                                   /////
////////////////////////////////////////////////////////////////////////////////////////

struct PWM
{
  
     byte pwmPin;
     byte pwmStatus;
     byte pwmHOn;
     byte pwmMOn;
     byte pwmSOn;
     byte pwmHOff;
     byte pwmMOff;
     byte pwmSOff;
     byte pwmMin;
     byte pwmMax;
     byte pwmSr;
     byte pwmSs;
     byte pwmKeepLight;
     boolean pwmInvert;
}


byte pwm(byte pwmPin, byte pwmStatus, byte pwmHOn, byte pwmMOn, byte pwmSOn, byte pwmHOff, byte pwmMOff, byte pwmSOff, byte pwmMin, byte pwmMax, byte pwmSr, byte pwmSs, byte pwmKeepLight, boolean pwmInvert) {
    byte low = 0, high = 255;

    if (pwmInvert) {
        low = 255;
        high = 0;
    }
    if (!pwmInvert) {
        low = 0;
        high = 255;
    }

    if (pwmStatus == 0) {
        analogWrite(pwmPin, map(0, 0, 255, low, high));
        return map(0, 0, 255, low, high);
    }

    if (forceOFF == true)
    {
        analogWrite(pwmPin, map(0, 0, 255, low, high));
        return map(0, 0, 255, low, high);
    }

    if (forceNight == true) {
        if (pwmKeepLight == true)
        {
  
          analogWrite(pwmPin, map(pwmMin, 0, 255, low, high));
            return map(pwmMin, 0, 255, low, high);
        }
        else
        {
            analogWrite(pwmPin, map(0, 0, 255, low, high));
            return map(0, 0, 255, low, high);
        }
    }

    if (forceAmbient == true) {
        analogWrite(pwmPin, map(AmbientPercent, 0, 100, low, high));
        return map(AmbientPercent, 0, 255, low, high);
    }

    long pwmOn = (long(pwmHOn)*3600) + (long(pwmMOn)*60) + long(pwmSOn);
    long pwmOff = (long(pwmHOff)*3600) + (long(pwmMOff)*60) + long(pwmSOff);

    boolean state = false;

    if (pwmOn < pwmOff) { // Normalnie  
        if (currentTimeSec >= pwmOn && currentTimeSec < pwmOff) {
            state = true;
        }
    }

    if (pwmOn > pwmOff) { // Zasuwaj dookola
        if (currentTimeSec >= pwmOn && currentTimeSec <= 86400) {
            state = true;
        }
        if (currentTimeSec >= 0 && currentTimeSec < pwmOff) {
            state = true;
        }
    }




    if (!state && !pwmKeepLight) {
        analogWrite(pwmPin, map(0, 0, 255, low, high));
        return map(0, 0, 255, low, high);
    }
    if (!state && pwmKeepLight) {
        if (pwmMin > 1 && pwmMin < 3) {
            pwmMin = 1;
        }
        analogWrite(pwmPin, map(pwmMin, 0, 255, low, high));
        return map(pwmMin, 0, 255, low, high);
    }



    ////////////////////////////////////
    ////  Swity 
    ///////////////////////////////////
    float jump;
    float sunriseLenght = pwmSr * 60;
    long elapsed;
    long pwmSunriseStop = pwmOn + sunriseLenght;
    long pwm;
    if (sunriseLenght == 0) {
        sunriseLenght = 1;
    }
    jump = ((pwmMax - pwmMin) / sunriseLenght);

    if (pwmSunriseStop < 86399 && currentTimeSec >= pwmOn && currentTimeSec <= pwmSunriseStop) {

        elapsed = sunriseLenght - (sunriseLenght - (currentTimeSec - pwmOn));
        pwm = pwmMin + (elapsed * jump);
        analogWrite(pwmPin, map(pwm, 0, 255, low, high));
        return map(pwm, 0, 255, low, high);
    }

    if ((pwmSunriseStop > 86399 && currentTimeSec >= pwmOn && currentTimeSec <= 86399) // przez zero
            || pwmSunriseStop > 86399 && currentTimeSec >= 0 && (currentTimeSec < (pwmSunriseStop - 86399))) {
        if (currentTimeSec >= pwmOn && currentTimeSec <= 86399) {
            elapsed = (sunriseLenght - (sunriseLenght - (currentTimeSec - pwmOn)));
        }
        else {
            elapsed = sunriseLenght - (sunriseLenght - (86399 - pwmOn + currentTimeSec));
        }

        pwm = pwmMin + (elapsed * jump);
        analogWrite(pwmPin, map(pwm, 0, 255, low, high));
        return map(pwm, 0, 255, low, high);
    }


    ///////////////////////
    ///// Zmierzchy ///////
    ///////////////////////
    float sunsetLenght = pwmSs * 60;
    float sunsetStart;


    if ((pwmOff - sunsetLenght) >= 0) {
        sunsetStart = pwmOff - sunsetLenght;
    } else {
        sunsetStart = 86400 + (pwmOff - sunsetLenght);
    }


    if (sunsetStart < pwmOff && sunsetStart >= 0 && currentTimeSec >= sunsetStart && currentTimeSec <= pwmOff) { // Normalnie
        elapsed = (currentTimeSec - sunsetStart);
        if (sunsetLenght == 0) {
            sunsetLenght = 1;
        }
        jump = (pwmMax - pwmMin) / sunsetLenght;
        pwm = pwmMax - (elapsed * jump);
        analogWrite(pwmPin, map(pwm, 0, 255, low, high));
        return map(pwm, 0, 255, low, high);
    }


    if (sunsetStart > pwmOff && currentTimeSec >= sunsetStart && currentTimeSec <= 86400) {
        elapsed = (currentTimeSec - sunsetStart);
        if (sunsetLenght == 0) {
            sunsetLenght = 1;
        }
        jump = (pwmMax - pwmMin) / sunsetLenght;
        pwm = pwmMax - (elapsed * jump);
        analogWrite(pwmPin, map(pwm, 0, 255, low, high));
        return map(pwm, 0, 255, low, high);
    }

    if (sunsetStart > pwmOff && currentTimeSec >= 0 && currentTimeSec < pwmOff) {
        elapsed = (86400 - sunsetStart) + currentTimeSec;
        if (sunsetLenght == 0) {
            sunsetLenght = 1;
        }
        jump = (pwmMax - pwmMin) / sunsetLenght;
        pwm = pwmMax - (elapsed * jump);
        analogWrite(pwmPin, map(pwm, 0, 255, low, high));
        return map(pwm, 0, 255, low, high);
    }


    //// Jak sie nic nie zlapalo, to wypal oczy
    analogWrite(pwmPin, map(pwmMax, 0, 255, low, high));
    return map(pwmMax, 0, 255, low, high);
}
////////////// 

////////////////////////////////////////////////////////////////////////////////////////
/////                             Analysis                                         /////
////////////////////////////////////////////////////////////////////////////////////////

boolean commandAnalysis(char cmdOutputArray[64]) {

    unsigned int val[64];
    char *cmdVal;
    int i = 0;

    cmdVal = strtok(cmdOutputArray, ",");

    while (cmdVal) {
        val[i++] = atoi(cmdVal);
        cmdVal = strtok(NULL, ",");
    }



    ///////////////////////////////////////////////////////////////////////////////////////
    /////                        PWM - 1, EPROM                                       ///// 
    ///////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 31) { //      1          2         3            4        5          6          7         8        9          10        11       12         13
        byte ePwm1Pin, ePwm1Status, ePwm1HOn, ePwm1MOn, ePwm1SOn, ePwm1HOff, ePwm1MOff, ePwm1SOff, ePwm1Min, ePwm1Max, ePwm1Sr, ePwm1Ss, ePwm1KeepLight;

        if (val[1] >= 0 && val[1] <= 99) {
            ePwm1Pin = val[1];
        } else {
            return false;
        } // ePwm1Pin -       1 
        if (val[2] >= 0 && val[2] <= 1) {
            ePwm1Status = val[2];
        } else {
            return false;
        } // ePwm1Status -    2
        if (val[3] >= 0 && val[3] <= 23) {
            ePwm1HOn = val[3];
        } else {
            return false;
        } // ePwm1HOn -       3
        if (val[4] >= 0 && val[4] <= 59) {
            ePwm1MOn = val[4];
        } else {
            return false;
        } // ePwm1MOn -       4
        if (val[5] >= 0 && val[5] <= 59) {
            ePwm1SOn = val[5];
        } else {
            return false;
        } // ePwm1SOn -       5
        if (val[6] >= 0 && val[6] <= 23) {
            ePwm1HOff = val[6];
        } else {
            return false;
        } // ePwm1HOff -      6
        if (val[7] >= 0 && val[7] <= 59) {
            ePwm1MOff = val[7];
        } else {
            return false;
        } // ePwm1MOff -      7
        if (val[8] >= 0 && val[8] <= 59) {
            ePwm1SOff = val[8];
        } else {
            return false;
        } // ePwm1SOff -      8
        if (val[9] >= 0 && val[9] <= 255) {
            ePwm1Min = val[9];
        } else {
            return false;
        } // ePwm1Min -       9
        if (val[10] >= 0 && val[10] <= 255) {
            ePwm1Max = val[10];
        } else {
            return false;
        } // ePwm1Max -       10
        if (val[11] >= 0 && val[11] <= 255) {
            ePwm1Sr = val[11];
        } else {
            return false;
        } // ePwm1Sr -        11
        if (val[12] >= 0 && val[12] <= 255) {
            ePwm1Ss = val[12];
        } else {
            return false;
        } // ePwm1Ss -        12
        if (val[13] >= 0 && val[13] <= 1) {
            ePwm1KeepLight = val[13];
        } else {
            return false;
        } // ePwm1KeepLight - 13

        EEPROM.write(281, ePwm1Pin);
        EEPROM.write(282, ePwm1Status);
        EEPROM.write(283, ePwm1HOn);
        EEPROM.write(284, ePwm1MOn);
        EEPROM.write(285, ePwm1SOn);
        EEPROM.write(286, ePwm1HOff);
        EEPROM.write(287, ePwm1MOff);
        EEPROM.write(288, ePwm1SOff);
        EEPROM.write(289, ePwm1Min);
        EEPROM.write(290, ePwm1Max);
        EEPROM.write(291, ePwm1Sr);
        EEPROM.write(292, ePwm1Ss);
        EEPROM.write(293, ePwm1KeepLight);
    }


    ///////////////////////////////////////////////////////////////////////////////////////
    /////                        PWM - 2, EPROM                                       ///// 
    ///////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 32) { //      1          2         3            4        5          6          7         8        9          10        11       12         13
        byte ePwm2Pin, ePwm2Status, ePwm2HOn, ePwm2MOn, ePwm2SOn, ePwm2HOff, ePwm2MOff, ePwm2SOff, ePwm2Min, ePwm2Max, ePwm2Sr, ePwm2Ss, ePwm2KeepLight;

        if (val[1] >= 0 && val[1] <= 99) {
            ePwm2Pin = val[1];
        } else {
            return false;
        } // ePwm2Pin -       1 
        if (val[2] >= 0 && val[2] <= 1) {
            ePwm2Status = val[2];
        } else {
            return false;
        } // ePwm2Status -    2
        if (val[3] >= 0 && val[3] <= 23) {
            ePwm2HOn = val[3];
        } else {
            return false;
        } // ePwm2HOn -       3
        if (val[4] >= 0 && val[4] <= 59) {
            ePwm2MOn = val[4];
        } else {
            return false;
        } // ePwm2MOn -       4
        if (val[5] >= 0 && val[5] <= 59) {
            ePwm2SOn = val[5];
        } else {
            return false;
        } // ePwm2SOn -       5
        if (val[6] >= 0 && val[6] <= 23) {
            ePwm2HOff = val[6];
        } else {
            return false;
        } // ePwm2HOff -      6
        if (val[7] >= 0 && val[7] <= 59) {
            ePwm2MOff = val[7];
        } else {
            return false;
        } // ePwm2MOff -      7
        if (val[8] >= 0 && val[8] <= 59) {
            ePwm2SOff = val[8];
        } else {
            return false;
        } // ePwm2SOff -      8
        if (val[9] >= 0 && val[9] <= 255) {
            ePwm2Min = val[9];
        } else {
            return false;
        } // ePwm2Min -       9
        if (val[10] >= 0 && val[10] <= 255) {
            ePwm2Max = val[10];
        } else {
            return false;
        } // ePwm2Max -       10
        if (val[11] >= 0 && val[11] <= 255) {
            ePwm2Sr = val[11];
        } else {
            return false;
        } // ePwm2Sr -        11
        if (val[12] >= 0 && val[12] <= 255) {
            ePwm2Ss = val[12];
        } else {
            return false;
        } // ePwm2Ss -        12
        if (val[13] >= 0 && val[13] <= 1) {
            ePwm2KeepLight = val[13];
        } else {
            return false;
        } // ePwm2KeepLight - 13

        EEPROM.write(301, ePwm2Pin);
        EEPROM.write(302, ePwm2Status);
        EEPROM.write(303, ePwm2HOn);
        EEPROM.write(304, ePwm2MOn);
        EEPROM.write(305, ePwm2SOn);
        EEPROM.write(306, ePwm2HOff);
        EEPROM.write(307, ePwm2MOff);
        EEPROM.write(308, ePwm2SOff);
        EEPROM.write(309, ePwm2Min);
        EEPROM.write(310, ePwm2Max);
        EEPROM.write(311, ePwm2Sr);
        EEPROM.write(312, ePwm2Ss);
        EEPROM.write(313, ePwm2KeepLight);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /////                        PWM - 3, EPROM                                       ///// 
    ///////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 33) { //      1          2         3            4        5          6          7         8        9          10        11       12         13
        byte ePwm3Pin, ePwm3Status, ePwm3HOn, ePwm3MOn, ePwm3SOn, ePwm3HOff, ePwm3MOff, ePwm3SOff, ePwm3Min, ePwm3Max, ePwm3Sr, ePwm3Ss, ePwm3KeepLight;

        if (val[1] >= 0 && val[1] <= 99) {
            ePwm3Pin = val[1];
        } else {
            return false;
        } // ePwm3Pin -       1 
        if (val[2] >= 0 && val[2] <= 1) {
            ePwm3Status = val[2];
        } else {
            return false;
        } // ePwm3Status -    2
        if (val[3] >= 0 && val[3] <= 23) {
            ePwm3HOn = val[3];
        } else {
            return false;
        } // ePwm3HOn -       3
        if (val[4] >= 0 && val[4] <= 59) {
            ePwm3MOn = val[4];
        } else {
            return false;
        } // ePwm3MOn -       4
        if (val[5] >= 0 && val[5] <= 59) {
            ePwm3SOn = val[5];
        } else {
            return false;
        } // ePwm3SOn -       5
        if (val[6] >= 0 && val[6] <= 23) {
            ePwm3HOff = val[6];
        } else {
            return false;
        } // ePwm3HOff -      6
        if (val[7] >= 0 && val[7] <= 59) {
            ePwm3MOff = val[7];
        } else {
            return false;
        } // ePwm3MOff -      7
        if (val[8] >= 0 && val[8] <= 59) {
            ePwm3SOff = val[8];
        } else {
            return false;
        } // ePwm3SOff -      8
        if (val[9] >= 0 && val[9] <= 255) {
            ePwm3Min = val[9];
        } else {
            return false;
        } // ePwm3Min -       9
        if (val[10] >= 0 && val[10] <= 255) {
            ePwm3Max = val[10];
        } else {
            return false;
        } // ePwm3Max -       10
        if (val[11] >= 0 && val[11] <= 255) {
            ePwm3Sr = val[11];
        } else {
            return false;
        } // ePwm3Sr -        11
        if (val[12] >= 0 && val[12] <= 255) {
            ePwm3Ss = val[12];
        } else {
            return false;
        } // ePwm3Ss -        12
        if (val[13] >= 0 && val[13] <= 1) {
            ePwm3KeepLight = val[13];
        } else {
            return false;
        } // ePwm3KeepLight - 13


        EEPROM.write(331, ePwm3Pin);
        EEPROM.write(332, ePwm3Status);
        EEPROM.write(333, ePwm3HOn);
        EEPROM.write(334, ePwm3MOn);
        EEPROM.write(335, ePwm3SOn);
        EEPROM.write(336, ePwm3HOff);
        EEPROM.write(337, ePwm3MOff);
        EEPROM.write(338, ePwm3SOff);
        EEPROM.write(339, ePwm3Min);
        EEPROM.write(340, ePwm3Max);
        EEPROM.write(341, ePwm3Sr);
        EEPROM.write(342, ePwm3Ss);
        EEPROM.write(343, ePwm3KeepLight);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /////                        PWM - 4, EPROM                                       ///// 
    ///////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 90) { //      1          2         3            4        5          6          7         8        9          10        11       12         13
        byte ePwm4Pin, ePwm4Status, ePwm4HOn, ePwm4MOn, ePwm4SOn, ePwm4HOff, ePwm4MOff, ePwm4SOff, ePwm4Min, ePwm4Max, ePwm4Sr, ePwm4Ss, ePwm4KeepLight;

        if (val[1] >= 0 && val[1] <= 99) {
            ePwm4Pin = val[1];
        } else {
            return false;
        } // ePwm4Pin -       1 
        if (val[2] >= 0 && val[2] <= 1) {
            ePwm4Status = val[2];
        } else {
            return false;
        } // ePwm4Status -    2
        if (val[3] >= 0 && val[3] <= 23) {
            ePwm4HOn = val[3];
        } else {
            return false;
        } // ePwm4HOn -       3
        if (val[4] >= 0 && val[4] <= 59) {
            ePwm4MOn = val[4];
        } else {
            return false;
        } // ePwm4MOn -       4
        if (val[5] >= 0 && val[5] <= 59) {
            ePwm4SOn = val[5];
        } else {
            return false;
        } // ePwm4SOn -       5
        if (val[6] >= 0 && val[6] <= 23) {
            ePwm4HOff = val[6];
        } else {
            return false;
        } // ePwm4HOff -      6
        if (val[7] >= 0 && val[7] <= 59) {
            ePwm4MOff = val[7];
        } else {
            return false;
        } // ePwm4MOff -      7
        if (val[8] >= 0 && val[8] <= 59) {
            ePwm4SOff = val[8];
        } else {
            return false;
        } // ePwm4SOff -      8
        if (val[9] >= 0 && val[9] <= 255) {
            ePwm4Min = val[9];
        } else {
            return false;
        } // ePwm4Min -       9
        if (val[10] >= 0 && val[10] <= 255) {
            ePwm4Max = val[10];
        } else {
            return false;
        } // ePwm4Max -       10
        if (val[11] >= 0 && val[11] <= 255) {
            ePwm4Sr = val[11];
        } else {
            return false;
        } // ePwm4Sr -        11
        if (val[12] >= 0 && val[12] <= 255) {
            ePwm4Ss = val[12];
        } else {
            return false;
        } // ePwm4Ss -        12
        if (val[13] >= 0 && val[13] <= 1) {
            ePwm4KeepLight = val[13];
        } else {
            return false;
        } // ePwm4KeepLight - 13

        EEPROM.write(371, ePwm4Pin);
        EEPROM.write(372, ePwm4Status);
        EEPROM.write(373, ePwm4HOn);
        EEPROM.write(374, ePwm4MOn);
        EEPROM.write(375, ePwm4SOn);
        EEPROM.write(376, ePwm4HOff);
        EEPROM.write(377, ePwm4MOff);
        EEPROM.write(378, ePwm4SOff);
        EEPROM.write(379, ePwm4Min);
        EEPROM.write(380, ePwm4Max);
        EEPROM.write(381, ePwm4Sr);
        EEPROM.write(382, ePwm4Ss);
        EEPROM.write(383, ePwm4KeepLight);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /////                        PWM - 5, EPROM                                       ///// 
    ///////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 91) { //      1          2         3            4        5          6          7         8        9          10        11       12         13
        byte ePwm5Pin, ePwm5Status, ePwm5HOn, ePwm5MOn, ePwm5SOn, ePwm5HOff, ePwm5MOff, ePwm5SOff, ePwm5Min, ePwm5Max, ePwm5Sr, ePwm5Ss, ePwm5KeepLight;

        if (val[1] >= 0 && val[1] <= 99) {
            ePwm5Pin = val[1];
        } else {
            return false;
        } // ePwm5Pin -       1 
        if (val[2] >= 0 && val[2] <= 1) {
            ePwm5Status = val[2];
        } else {
            return false;
        } // ePwm5Status -    2
        if (val[3] >= 0 && val[3] <= 23) {
            ePwm5HOn = val[3];
        } else {
            return false;
        } // ePwm5HOn -       3
        if (val[4] >= 0 && val[4] <= 59) {
            ePwm5MOn = val[4];
        } else {
            return false;
        } // ePwm5MOn -       4
        if (val[5] >= 0 && val[5] <= 59) {
            ePwm5SOn = val[5];
        } else {
            return false;
        } // ePwm5SOn -       5
        if (val[6] >= 0 && val[6] <= 23) {
            ePwm5HOff = val[6];
        } else {
            return false;
        } // ePwm5HOff -      6
        if (val[7] >= 0 && val[7] <= 59) {
            ePwm5MOff = val[7];
        } else {
            return false;
        } // ePwm5MOff -      7
        if (val[8] >= 0 && val[8] <= 59) {
            ePwm5SOff = val[8];
        } else {
            return false;
        } // ePwm5SOff -      8
        if (val[9] >= 0 && val[9] <= 255) {
            ePwm5Min = val[9];
        } else {
            return false;
        } // ePwm5Min -       9
        if (val[10] >= 0 && val[10] <= 255) {
            ePwm5Max = val[10];
        } else {
            return false;
        } // ePwm5Max -       10
        if (val[11] >= 0 && val[11] <= 255) {
            ePwm5Sr = val[11];
        } else {
            return false;
        } // ePwm5Sr -        11
        if (val[12] >= 0 && val[12] <= 255) {
            ePwm5Ss = val[12];
        } else {
            return false;
        } // ePwm5Ss -        12
        if (val[13] >= 0 && val[13] <= 1) {
            ePwm5KeepLight = val[13];
        } else {
            return false;
        } // ePwm5KeepLight - 13

        EEPROM.write(391, ePwm5Pin);
        EEPROM.write(392, ePwm5Status);
        EEPROM.write(393, ePwm5HOn);
        EEPROM.write(394, ePwm5MOn);
        EEPROM.write(395, ePwm5SOn);
        EEPROM.write(396, ePwm5HOff);
        EEPROM.write(397, ePwm5MOff);
        EEPROM.write(398, ePwm5SOff);
        EEPROM.write(399, ePwm5Min);
        EEPROM.write(400, ePwm5Max);
        EEPROM.write(401, ePwm5Sr);
        EEPROM.write(402, ePwm5Ss);
        EEPROM.write(403, ePwm5KeepLight);
    }


    ///////////////////////////////////////////////////////////////////////////////////////
    /////                        PWM - 6, EPROM                                       ///// 
    ///////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 92) { //      1          2         3            4        5          6          7         8        9          10        11       12         13
        byte ePwm6Pin, ePwm6Status, ePwm6HOn, ePwm6MOn, ePwm6SOn, ePwm6HOff, ePwm6MOff, ePwm6SOff, ePwm6Min, ePwm6Max, ePwm6Sr, ePwm6Ss, ePwm6KeepLight;

        if (val[1] >= 0 && val[1] <= 99) {
            ePwm6Pin = val[1];
        } else {
            return false;
        } // ePwm6Pin -       1 
        if (val[2] >= 0 && val[2] <= 1) {
            ePwm6Status = val[2];
        } else {
            return false;
        } // ePwm6Status -    2
        if (val[3] >= 0 && val[3] <= 23) {
            ePwm6HOn = val[3];
        } else {
            return false;
        } // ePwm6HOn -       3
        if (val[4] >= 0 && val[4] <= 59) {
            ePwm6MOn = val[4];
        } else {
            return false;
        } // ePwm6MOn -       4
        if (val[5] >= 0 && val[5] <= 59) {
            ePwm6SOn = val[5];
        } else {
            return false;
        } // ePwm6SOn -       5
        if (val[6] >= 0 && val[6] <= 23) {
            ePwm6HOff = val[6];
        } else {
            return false;
        } // ePwm6HOff -      6
        if (val[7] >= 0 && val[7] <= 59) {
            ePwm6MOff = val[7];
        } else {
            return false;
        } // ePwm6MOff -      7
        if (val[8] >= 0 && val[8] <= 59) {
            ePwm6SOff = val[8];
        } else {
            return false;
        } // ePwm6SOff -      8
        if (val[9] >= 0 && val[9] <= 255) {
            ePwm6Min = val[9];
        } else {
            return false;
        } // ePwm6Min -       9
        if (val[10] >= 0 && val[10] <= 255) {
            ePwm6Max = val[10];
        } else {
            return false;
        } // ePwm6Max -       10
        if (val[11] >= 0 && val[11] <= 255) {
            ePwm6Sr = val[11];
        } else {
            return false;
        } // ePwm6Sr -        11
        if (val[12] >= 0 && val[12] <= 255) {
            ePwm6Ss = val[12];
        } else {
            return false;
        } // ePwm6Ss -        12
        if (val[13] >= 0 && val[13] <= 1) {
            ePwm6KeepLight = val[13];
        } else {
            return false;
        } // ePwm6KeepLight - 13

        EEPROM.write(411, ePwm6Pin);
        EEPROM.write(412, ePwm6Status);
        EEPROM.write(413, ePwm6HOn);
        EEPROM.write(414, ePwm6MOn);
        EEPROM.write(415, ePwm6SOn);
        EEPROM.write(416, ePwm6HOff);
        EEPROM.write(417, ePwm6MOff);
        EEPROM.write(418, ePwm6SOff);
        EEPROM.write(419, ePwm6Min);
        EEPROM.write(420, ePwm6Max);
        EEPROM.write(421, ePwm6Sr);
        EEPROM.write(422, ePwm6Ss);
        EEPROM.write(423, ePwm6KeepLight);

    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    /////                        Set Date                                                   /////
    /////////////////////////////////////////////////////////////////////////////////////////////
    static byte setHour, setMinute, setSecond, setYear, setMonth, setDay;

    if (val[0] == 40) {
        if (val[1] >= 0 && val[1] <= 23) {
            setHour = val[1];
        } else {
            return false;
        } // setHour -   1
        if (val[2] >= 0 && val[2] <= 59) {
            setMinute = val[2];
        } else {
            return false;
        } // setMinute - 2
        if (val[3] >= 0 && val[3] <= 59) {
            setSecond = val[3];
        } else {
            return false;
        } // setSecond - 3
        if (val[4] >= 15 && val[4] <= 99) {
            setYear = val[4] + 2000;
        } else {
            return false;
        } // setYear -   4
        if (val[5] >= 1 && val[5] <= 99) {
            setMonth = val[5];
        } else {
            return false;
        } // setMonth -  5
        if (val[6] >= 1 && val[6] <= 31) {
            setDay = val[6];
        } else {
            return false;
        } // setDay -    5

        setTime(setHour, setMinute, setSecond, setDay, setMonth, setYear); // godz, min, sek, dzien, miesiac, rok

        RTC.set(now());
        return true;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////
    /////                       Get Settings                                              /////
    ///////////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 66) {
        int eAddress = 0;
       // Serial.write("*66,");
        bluetooth.write("*66,");

        while (eAddress <= 512) {
//            Serial.print(EEPROM.read(eAddress));
            bluetooth.print(EEPROM.read(eAddress));
//            Serial.write(",");
            bluetooth.write(",");
            eAddress++;
        }
//        Serial.write("#\n");
        bluetooth.write("#\n");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /////                       Get Date                                                  /////
    ///////////////////////////////////////////////////////////////////////////////////////////
    if (val[0] == 83) {
//        Serial.print("*83");
//        Serial.write(",");
//        Serial.print(tm.Hour);
//        Serial.write(",");
//        Serial.print(tm.Minute);
//        Serial.write(",");
//        Serial.print(tm.Second);
//        Serial.write(",");
//        Serial.print(tmYearToCalendar(tm.Year));
//        Serial.write(",");
//        Serial.print(tm.Month);
//        Serial.write(",");
//        Serial.print(tm.Day);
//        Serial.println();

        bluetooth.print("*83");
        bluetooth.write(",");
        bluetooth.print(tm.Hour);
        bluetooth.write(",");
        bluetooth.print(tm.Minute);
        bluetooth.write(",");
        bluetooth.print(tm.Second);
        bluetooth.write(",");
        bluetooth.print(tmYearToCalendar(tm.Year));
        bluetooth.write(",");
        bluetooth.print(tm.Month);
        bluetooth.write(",");
        bluetooth.print(tm.Day);
        bluetooth.println();

    }

    if (val[0] == 80) {
//        Serial.println("AQma LED Control, 2.0.1 - BT");
        bluetooth.println("AQma LED Control, 2.0.1 - BT");
    }


}




///////////////////////////////////////////////////////////////////////////
/////                       eEpromRead                                ///// 
///////////////////////////////////////////////////////////////////////////

void eEpromRead() {

    // PWM - 1
    //pwm1Pin = EEPROM.read(281); 
    pwm1Status = EEPROM.read(282);
    pwm1HOn = EEPROM.read(283);
    pwm1MOn = EEPROM.read(284);
    pwm1SOn = EEPROM.read(285);
    pwm1HOff = EEPROM.read(286);
    pwm1MOff = EEPROM.read(287);
    pwm1SOff = EEPROM.read(288);
    pwm1Min = EEPROM.read(289);
    pwm1Max = EEPROM.read(290);
    pwm1Sr = EEPROM.read(291);
    pwm1Ss = EEPROM.read(292);
    pwm1KeepLight = EEPROM.read(293);

    // PWM - 2
    //pwm2Pin = EEPROM.read(301); 
    pwm2Status = EEPROM.read(302);
    pwm2HOn = EEPROM.read(303);
    pwm2MOn = EEPROM.read(304);
    pwm2SOn = EEPROM.read(305);
    pwm2HOff = EEPROM.read(306);
    pwm2MOff = EEPROM.read(307);
    pwm2SOff = EEPROM.read(308);
    pwm2Min = EEPROM.read(309);
    pwm2Max = EEPROM.read(310);
    pwm2Sr = EEPROM.read(311);
    pwm2Ss = EEPROM.read(312);
    pwm2KeepLight = EEPROM.read(313);

    // PWM - 3
    //pwm3Pin = EEPROM.read(331); 
    pwm3Status = EEPROM.read(332);
    pwm3HOn = EEPROM.read(333);
    pwm3MOn = EEPROM.read(334);
    pwm3SOn = EEPROM.read(335);
    pwm3HOff = EEPROM.read(336);
    pwm3MOff = EEPROM.read(337);
    pwm3SOff = EEPROM.read(338);
    pwm3Min = EEPROM.read(339);
    pwm3Max = EEPROM.read(340);
    pwm3Sr = EEPROM.read(341);
    pwm3Ss = EEPROM.read(342);
    pwm3KeepLight = EEPROM.read(343);

    // PWM - 4
    //pwm4Pin = EEPROM.read(371); 
    pwm4Status = EEPROM.read(372);
    pwm4HOn = EEPROM.read(373);
    pwm4MOn = EEPROM.read(374);
    pwm4SOn = EEPROM.read(375);
    pwm4HOff = EEPROM.read(376);
    pwm4MOff = EEPROM.read(377);
    pwm4SOff = EEPROM.read(378);
    pwm4Min = EEPROM.read(379);
    pwm4Max = EEPROM.read(380);
    pwm4Sr = EEPROM.read(381);
    pwm4Ss = EEPROM.read(382);
    pwm4KeepLight = EEPROM.read(383);

    // PWM - 5
    //pwm5Pin = EEPROM.read(391); 
    pwm5Status = EEPROM.read(392);
    pwm5HOn = EEPROM.read(393);
    pwm5MOn = EEPROM.read(394);
    pwm5SOn = EEPROM.read(395);
    pwm5HOff = EEPROM.read(396);
    pwm5MOff = EEPROM.read(397);
    pwm5SOff = EEPROM.read(398);
    pwm5Min = EEPROM.read(399);
    pwm5Max = EEPROM.read(400);
    pwm5Sr = EEPROM.read(401);
    pwm5Ss = EEPROM.read(402);
    pwm5KeepLight = EEPROM.read(403);

    // PWM - 6
    //pwm6Pin = EEPROM.read(411); 
    pwm6Status = EEPROM.read(412);
    pwm6HOn = EEPROM.read(413);
    pwm6MOn = EEPROM.read(414);
    pwm6SOn = EEPROM.read(415);
    pwm6HOff = EEPROM.read(416);
    pwm6MOff = EEPROM.read(417);
    pwm6SOff = EEPROM.read(418);
    pwm6Min = EEPROM.read(419);
    pwm6Max = EEPROM.read(420);
    pwm6Sr = EEPROM.read(421);
    pwm6Ss = EEPROM.read(422);
    pwm6KeepLight = EEPROM.read(423);
}

////////////////////////////////////////////////////////////////////////////////////////
/////                   Silky Smooth Power On                                      /////
////////////////////////////////////////////////////////////////////////////////////////
void silkySmoothPowerOn(int silkySmoothTime, unsigned long currentMillis)
{
 static unsigned int silkySmoothCounter = 0;
 static unsigned long silkySmoothLastCounterTime = 0;
 silkySmoothTime = silkySmoothTime; 
  
  if (justTurnedOn && silkySmoothCounter <= silkySmoothTime) 
    {
    if ( currentMillis < 1000 ) { pwm1Max = 0; pwm2Max = 0; pwm3Max = 0; pwm4Max = 0; pwm5Max = 0; pwm6Max = 0; }
   
    if ( (currentMillis - silkySmoothLastCounterTime) >= 1000 ) 
       { 
        silkySmoothCounter++; 
        silkySmoothLastCounterTime = currentMillis; 
        pwm1Max = ((EEPROM.read(290) / silkySmoothTime)*silkySmoothCounter);
        pwm2Max = ((EEPROM.read(310) / silkySmoothTime)*silkySmoothCounter);
        pwm3Max = ((EEPROM.read(340) / silkySmoothTime)*silkySmoothCounter);
        pwm4Max = ((EEPROM.read(380) / silkySmoothTime)*silkySmoothCounter);
        pwm5Max = ((EEPROM.read(400) / silkySmoothTime)*silkySmoothCounter);
        pwm6Max = ((EEPROM.read(420) / silkySmoothTime)*silkySmoothCounter); 
       } 

        if ( silkySmoothCounter == silkySmoothTime) 
           {
            justTurnedOn = false;
            eEpromRead();
           }
    
    }
    
 }




