# AquaLed (en)

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

####PINS SETUP:
#####To use arduino built-in PWM pins (arduino ATmega328 or ATmega168 has 6 PWM channels, MEGA 2560 has 15 PWM channels):
```
// PWM
#define PWMS 6
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

#####To use I2C PWM module (PCA9635, PCA9685 etc. modules) pins:
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


####Modules (uncomment to disable). As for now bluetooth and nextion support does not fit into Arduino Nano/UNO etc. flash memory. Use arduino Mega or disable one module support.
```
// modules, uncomment to disable, comment to enable
#define NO_BLUETOOTH
//#define NO_NEXTION
//#define NO_I2C
//#define NO_TEMPERATURE
```
####Other
There are many other options for eg. timers resolution, most of them are pretty self-explanatory ;)

**Wiring:**

![Alt text](http://pifpaf.pl/ftp/aqualed/ss/wiring.jpg?"AquaLed wiring")



# AquaLed (pl)

Sterownik lampy LED PWM na Arduino z obsługą termometrów, chłodzenia i wyświetlacza LCD Nextion dla akwariów słodko i słonowodnych.

Możliwości:
- obsługa 6 ściemnialnych kanałów PWM dla modułów LED (driverów), z modułem I2C do 512 kanałów (maksymalnie 8 na wyswietlaczu)
- programowanie godzin świetenia z symulacją wschodów i zachodów słońca
- światło nocne
- obsługa 3 cyfrowych termometrów sterujących pracą 3 przekaźników (np. wentylatorów), np. dla lampy, lampy w sumpie, wody w akwarium
- ściemnianie/rozjaśnianie
- kompatybilność z AQma Led Control by Maqu http://magu.pl/aqma-led-control w zakresie programowania przez Bluetooth
- pełna konfiguracja i status za pomocą wyświetlacza Nextion LCD (przykłady na 2,8"), sterowanie przez dotyk.
- możliwość chwilowego nadpisania działającego programu z wyświetlacza: wyłączenie/wymuszenie światła nocnego/program zdefiniowany
- obsługa godziny i daty
- wygaszacz ekranu z godziną i temperaturą wody
- tryb test
- wszystkie ustawienia zapamiętywane w pamięci EEPROM
- działa z Arduino Nano lub wyższym modelem

Przykładowy program na wyświetlacz załączony.

**Wygląd ekranów na wyświetlaczu **

![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/0.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/1.jpg> "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/2.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/9.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/3.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/4.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/5.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/6.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/7.jpg? "AquaLed")
![Alt text](http://pifpaf.pl/ftp/aqualed/sspl/8.jpg? "AquaLed")

**Wymagania:**

**Sprzęt:**
- Arduino Nano lub wyższy
- moduł czasu rzeczywistego RTC
- Bluetooth (opcjonalnie)
- 3x termometry cyfrowe DS18B20
- 3x moduły przekaźników 5v
- moduł PCA9685 PWM i2c  (optionalny)
- wyświetlacz Nextion 2.8" LCD lub większy

**Biblioteki:**
- Arduino-Temperature-Control-Library (https://github.com/milesburton/Arduino-Temperature-Control-Library)
- DS1307RTC (https://github.com/PaulStoffregen/DS1307RTC)
- DS18B20 (https://github.com/nettigo/DS18B20)
- OneWire (https://github.com/PaulStoffregen/OneWire)
- Time (https://github.com/PaulStoffregen/Time)
- Adafruit-PWM-Servo-Driver-Library (https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)

**Oprogramowanie:**
- Nextion Editor (https://nextion.itead.cc/download.html)
- Arduino IDE lub atom.io

**Konfiguracja:**
Konfiguracja w pliku aqualed.h i nextion.h

####Ustawienia pinów PWM:
#####Jeśli chcesz używać wbudowanych w Arduino pinów PWM (arduino ATmega328 lub ATmega168 posiada wyjść 6 PWM, MEGA 2560 posiada 15 wyjść PWM):
```
// PWM
#define PWMS 6
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

#####Jeśli chcesz użyć modułu I2C (PCA9635, PCA9685):
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


####Moduły (odkomentuj żeby wyłączyć). Uwaga! Obsługa bluetooth i nextio nie mieści się jednocześnie w pamięcy flash Arduino Nano/Uno itd. wchodzi tylko do Arduino Mega lub podobnego.
```
// modules, uncomment to disable, comment to enable
#define NO_BLUETOOTH
//#define NO_NEXTION
//#define NO_I2C
//#define NO_TEMPERATURE
```
####Inne
W pliku konfiguracyjnym można ustawić wiele opcji w tym czasy działania timerów, rozdzielczość załączania przekaźników itd. Większość ustawień wynika z samych nazw stałych :)

**Podłączenie:**

![Alt text](http://pifpaf.pl/ftp/aqualed/ss/wiring.jpg?"AquaLed wiring")





