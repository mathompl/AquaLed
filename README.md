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

REQUIREMENTS:

Hardware:
- Arduino Nano or better
- RTC module 
- Bluetooth (optional)
- 3x DS18B20 digital thermometers 
- 3x 5v relays modules
- PCA9685 PWM i2c module (optional)
- Nextion 2.8" LCD


Libraries:
- Arduino-Temperature-Control-Library (https://github.com/milesburton/Arduino-Temperature-Control-Library) 
- DS1307RTC (https://github.com/PaulStoffregen/DS1307RTC)
- DS18B20 (https://github.com/nettigo/DS18B20)
- OneWire (https://github.com/PaulStoffregen/OneWire)
- Time (https://github.com/PaulStoffregen/Time)
- Adafruit-PWM-Servo-Driver-Library (https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)

Software:
- Nextion Editor (https://nextion.itead.cc/download.html)
- Arduino IDE or atom.io
 
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/0.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/1.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/2.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/9.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/3.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/4.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/5.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/6.jpg "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/ssen/7.jpg "AquaLed")

