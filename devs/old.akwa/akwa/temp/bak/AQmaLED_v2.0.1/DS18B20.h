#ifndef _DS18B20_h_
#define _DS18B20_h_

#include <Arduino.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <OneWire.h>

#define TEMP_ERROR -273.15f

// Pointer type to an array in flash memory of device address
#define FA( pgm_ptr ) ( reinterpret_cast< const __FlashStringHelper * >( pgm_ptr ) )

// Exceptions catcher
#define E(a) __check(a, __LINE__)

// Teperature value exception catcher
#define TE(a) __check(a != TEMP_ERROR, __LINE__)


void __check(bool value, uint16_t line);

class DS18B20
{
public:
  DS18B20(OneWire *oneWire);
  bool begin(uint8_t quality=12);
  bool request(void);
  bool request(uint8_t *address);
  bool request(const __FlashStringHelper *_address);

  bool available(void);
  float readTemperature(uint8_t *address);
  float readTemperature(const __FlashStringHelper *_address);

private:
  OneWire *_oneWire;
  uint8_t _quality;
  bool _samePowerType;
  bool _powerType;
  uint32_t _beginConversionTime;

  bool _sendCommand(uint8_t *address, uint8_t command);
  bool _sendQuality(uint8_t *address);
  bool _receivePowerType(uint8_t *address);
  void _readFlashAddress(const __FlashStringHelper *_address, uint8_t *address);
};
#endif
























