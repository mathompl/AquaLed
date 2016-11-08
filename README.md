# AquaLed

Arduino PWM LED lamp driver software with LCD support and temperature control for marine, reef or sweetwater aquariums.

Features:
- 6 channel PWM control of LED modules (drivers), with I2C module up to 512 (up to 8 channels on LCD)
- programmable light hours with sunsets and sunrises
- night light support
- 3 digital thermometers with asynchronous temperature read, for: water, lamp and addictional lamp or sump, controlling 3 separate  relays (cooling fans)
- customizable dimming, linear or logarithmic algorith
- bluetooth compatibility with AQma Led Control by Maqu http://magu.pl/aqma-led-control
- full configuration and status using Nextion 2.8" LCD with touch support, including: operational hours, sunset & sunrise hours, thermometers, dimming times, schedule, night mode etc.
- override programs: off, night and user accessible from LCD
- date/time support with RTC module
- screen saver with water temperature and time
- test mode
- all settings stored in EEPROM
- works on Arduino Nano

Example Nextion project attached. I dont have copyright for graphics used, use only as an example.

**Nextion LCD Screenshots**
 
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/0.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/1.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/2.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/9.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/3.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/4.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/5.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/6.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/7.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/8.jpg "AquaLed")

**REQUIREMENTS:**

**Hardware:**
- Arduino Nano or better
- RTC module 
- Bluetooth (optional)
- 3x DS18B20 digital thermometers 
- 3x 5v relays modules
- PCA9685 PWM i2c module (optional)
- Nextion 2.8" LCD

**Libraries:**
- Arduino-Temperature-Control-Library (https://github.com/milesburton/Arduino-Temperature-Control-Library) 
- DS1307RTC (https://github.com/PaulStoffregen/DS1307RTC)
- DS18B20 (https://github.com/nettigo/DS18B20)
- OneWire (https://github.com/PaulStoffregen/OneWire)
- Time (https://github.com/PaulStoffregen/Time)
- Adafruit-PWM-Servo-Driver-Library (https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)

**Software:**
- Nextion Editor (https://nextion.itead.cc/download.html)
- Arduino IDE or atom.io

**Configuration:**
Driver configuration is in file aqualed.h

##PINS SETUP:
###To use arduino built-in PWM pins:
```
// PWM
#define PWMS 8
#define PWM1_PIN  3
#define PWM2_PIN  5
#define PWM3_PIN  6
#define PWM4_PIN  9
#define PWM5_PIN  10
#define PWM6_PIN  11
#define PWM7_PIN  1
#define PWM8_PIN  2

#define PWM1_I2C  0
#define PWM2_I2C  0
#define PWM3_I2C  0
#define PWM4_I2C  0
#define PWM5_I2C  0
#define PWM6_I2C  0
#define PWM7_I2C  1
#define PWM8_I2C  1
```

###To use I2C PWM module pins:
```
// PWM pins config
#define PWMS 8
#define PWM1_PIN  0
#define PWM2_PIN  1
#define PWM3_PIN  2
#define PWM4_PIN  3
#define PWM5_PIN  4
#define PWM6_PIN  5
#define PWM7_PIN  6
#define PWM8_PIN  7

// 0 - Arduino PWM pins
// 1 - i2c PWM module, eg. PCA9685

#define PWM1_I2C  1
#define PWM2_I2C  1
#define PWM3_I2C  1
#define PWM4_I2C  1
#define PWM5_I2C  1
#define PWM6_I2C  1
#define PWM7_I2C  1
#define PWM8_I2C  1
```


##Modules (uncomment to disable)
```
// modules, uncomment to disable, comment to enable
#define NO_BLUETOOTH
//#define NO_NEXTION
//#define NO_I2C
//#define NO_TEMPERATURE
```
##Other
There are many other options for eg. timers resolution, most of them are pretty self-explanatory ;)

**Wiring:**

![Alt text](http://pifpaf.pl/ftp/aqualed/ss/wiring.jpg?"AquaLed wiring")


