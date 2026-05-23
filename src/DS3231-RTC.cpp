/**
 * @file    DS3231-RTC.cpp
 * @author  Frank Häfele
 * @date    31.03.2026
 * @version 1.2.0
 * @brief   Real-Time clock library based on Arduino Framework
 */

#include "DS3231-RTC.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

// These included for the DateTime class inclusion; will try to find a way to
// not need them in the future...
#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_uint8_t
#define pgm_read_uint8_t(addr) (*(addr))
#endif

// *****************************************
//  Static Functions only used in this file
// *****************************************

static const uint8_t daysInMonth[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static void safe_gmtime(const time_t *timestamp, struct tm *timestruct) {
#if defined(_WIN32)
   gmtime_s(timestruct, timestamp);
#else
   gmtime_r(timestamp, timestruct);
#endif
}

static void safe_localtime(const time_t *timestamp, struct tm *timestruct) {
#if defined(_WIN32)
   localtime_s(timestruct, timestamp);
#else
   localtime_r(timestamp, timestruct);
#endif
}

/**
 * @brief function which calculates if a year is a leap year
 *
 * @param year
 * @return true
 * @return false
 */
static bool isleapYear(const int16_t year) {
   // check if divisible by 4
   if (year % 4) {
      return false;
   }
   // only check OR (second condition), when first failed
   return (year % 100 || year % 400 == 0);
}

/**
 * @brief calculate the days since January 1 (0...365)
 *
 * @param year e.g.: 2022
 * @param month 1...12
 * @param day 1...31
 * @return int16_t
 */
static int16_t calcYearDay(const int16_t year, const int8_t month, const int8_t day) {
   uint16_t days = day - 1;
   for (uint8_t i = 1; i < month; ++i)
      days += pgm_read_uint8_t(daysInMonth + i - 1);
   if (month > 2 && isleapYear(year))
      ++days;
   return days;
}


#if DS3231_RTC_HAS_WIRE
DS3231::TwoWireAdapter::TwoWireAdapter(TwoWire *wire)
: _wire(wire)
{}

void DS3231::TwoWireAdapter::beginTransmission(uint8_t address) {
   _wire->beginTransmission(address);
}

size_t DS3231::TwoWireAdapter::write(uint8_t value) {
   return _wire->write(value);
}

uint8_t DS3231::TwoWireAdapter::endTransmission() {
   return _wire->endTransmission();
}

uint8_t DS3231::TwoWireAdapter::requestFrom(uint8_t address, uint8_t quantity) {
   return _wire->requestFrom(address, quantity);
}

int DS3231::TwoWireAdapter::read() {
   return _wire->read();
}

int DS3231::TwoWireAdapter::available() {
   return _wire->available();
}

void DS3231::TwoWireAdapter::begin() {
   if (_wire) {
      _wire->begin();
   }
}
#endif

#pragma region DateTime
DS3231::DateTime::DateTime (time_t unix_timestamp)
: _unix_timestamp{unix_timestamp}, _y2k_timestamp{static_cast<time_t>(unix_timestamp - UNIX_OFFSET)}
{
   safe_gmtime(&_unix_timestamp, &_tm);
}

DS3231::DateTime::DateTime(int16_t year, int8_t month, int8_t day, int8_t hour, int8_t min, int8_t sec, int8_t wday, int16_t yday, int16_t dst)
{
   _tm.tm_sec = sec;
   _tm.tm_min = min;
   _tm.tm_hour = hour;
   _tm.tm_mday = day;
   _tm.tm_mon = month-1;
   _tm.tm_year = year-1900;
   _tm.tm_wday = wday-1;
   _tm.tm_yday = yday;
   _tm.tm_isdst = dst;

   set_timstamps();
}

DS3231::DateTime::DateTime(const char *date, const char *time) {
   static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
   static char month_buff[4] = {'0','0','0','0'};
   int year, day;
#if defined(_MSC_VER)
   sscanf_s(date, "%3s %2d %4d", month_buff, static_cast<unsigned>(sizeof(month_buff)), &day, &year);
#else
   sscanf(date, "%s %2d %4d", month_buff, &day, &year);
#endif
   int month = static_cast<int>((strstr(month_names, month_buff) - month_names) / 3 + 1);
   _tm.tm_year = year-1900;
   _tm.tm_mon = month-1;
   _tm.tm_mday = day;
   uint8_t hour, min, sec;
#if defined(_MSC_VER)
   sscanf_s(time, "%hhu:%hhu:%hhu", &hour, &min, &sec);
#else
   sscanf(time, "%hhu:%hhu:%hhu", &hour, &min, &sec);
#endif
   _tm.tm_hour = hour;
   _tm.tm_min = min;
   _tm.tm_sec = sec;
   _tm.tm_yday = calcYearDay(year, month, day);
   set_timstamps();
}

void DS3231::DateTime::set_timstamps() {
#if defined (__AVR__)
   _y2k_timestamp = mktime(&_tm);
   _unix_timestamp = _y2k_timestamp + UNIX_OFFSET;
#else
   _unix_timestamp = mktime(&_tm);
   _y2k_timestamp = _unix_timestamp - UNIX_OFFSET;
#endif
}

size_t DS3231::DateTime::strf_DateTime(char *buffer, size_t buffersize, const char *formatSpec) {
   size_t len {strftime(buffer, buffersize, formatSpec, &_tm)};
   return len;
}
#pragma endregion DateTime

DS3231::DateTime DS3231::RTClib::now(BusInterface &bus) {
   bus.beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS);
   bus.write(0);
   bus.endTransmission();

   bus.requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 7);
   int8_t sec = DS3231_Tools::bcdToDec(static_cast<uint8_t>(bus.read()) & 0x7F);
   int8_t min = DS3231_Tools::bcdToDec(static_cast<uint8_t>(bus.read()) & 0x7F);
   
   // Read hour register and respect 12/24h mode (Bit 6)
   uint8_t hour_byte = static_cast<uint8_t>(bus.read());
   bool h12 = hour_byte & 0b01000000;
   int8_t hour;
   if (h12) {
      // 12-hour mode: only use bits 4-0 for hour value
      hour = DS3231_Tools::bcdToDec(hour_byte & 0b00011111);
   } else {
      // 24-hour mode: use bits 5-0 for hour value
      hour = DS3231_Tools::bcdToDec(hour_byte & 0b00111111);
   }
   
   int8_t wday = DS3231_Tools::bcdToDec(static_cast<uint8_t>(bus.read()) & 0x07) - 1;
   int8_t day = DS3231_Tools::bcdToDec(static_cast<uint8_t>(bus.read()) & 0x3F);
   int8_t month = DS3231_Tools::bcdToDec(static_cast<uint8_t>(bus.read()) & 0x1F);
   int16_t year = DS3231_Tools::bcdToDec(static_cast<uint8_t>(bus.read())) + 2000;
   int16_t yday = calcYearDay(year, month, day);
   int16_t dst = -1;

   return DateTime{year, month, day, hour, min, sec, wday, yday, dst};
}

#if DS3231_RTC_HAS_WIRE
DS3231::DateTime DS3231::RTClib::now(TwoWire &_Wire) {
   TwoWireAdapter adapter(&_Wire);
   return now(adapter);
}
#endif

#pragma region DS3231
#if DS3231_RTC_HAS_WIRE
DS3231::DS3231::DS3231()
: _wire_adapter(&Wire), _bus(&_wire_adapter)
{}

DS3231::DS3231::DS3231(TwoWire &twowire)
: _wire_adapter(&twowire), _bus(&_wire_adapter)
{}
#endif

DS3231::DS3231::DS3231(BusInterface &bus)
#if DS3231_RTC_HAS_WIRE
: _wire_adapter{nullptr}, _bus{&bus}
#else
: _bus{&bus}
#endif
{}

uint8_t DS3231::DS3231::getSecond() {
   selectRegister(0x00);
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getMinute() {
   selectRegister(0x01);
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getHour(bool& h12, bool& PM_time) {
   uint8_t temp_buffer = readRegisterRaw(0x02);
   uint8_t hour;

   h12 = temp_buffer & 0b01000000;
   if (h12) {
      PM_time = temp_buffer & 0b00100000;
      hour = DS3231_Tools::bcdToDec(temp_buffer & 0b00011111);
   }
   else {
      PM_time = 0;
      hour = DS3231_Tools::bcdToDec(temp_buffer & 0b00111111);
   }
   return hour;
}

uint8_t DS3231::DS3231::getDoW() {
   selectRegister(0x03);
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getDate() {
   selectRegister(0x04);
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getMonth(bool &century) {
   uint8_t temp_buffer = readRegisterRaw(0x05);
   century = temp_buffer & 0b10000000;
   return DS3231_Tools::bcdToDec(temp_buffer & 0b01111111);
}

uint8_t DS3231::DS3231::getYear() {
   selectRegister(0x06);
   return getRegisterValue();
}

void DS3231::DS3231::setEpoch(time_t epoch, bool flag_localtime) {
#if defined (__AVR__)
   epoch -= UNIX_OFFSET;
#endif
   struct tm tmnow;
   if (flag_localtime) {
      safe_localtime(&epoch, &tmnow);
   }
   else {
      safe_gmtime(&epoch, &tmnow);
   }
   setSecond(static_cast<uint8_t>(tmnow.tm_sec));
   setMinute(static_cast<uint8_t>(tmnow.tm_min));
   setHour(static_cast<uint8_t>(tmnow.tm_hour));
   setDoW(static_cast<uint8_t>(tmnow.tm_wday + 1U));
   setDate(static_cast<uint8_t>(tmnow.tm_mday));
   setMonth(static_cast<uint8_t>(tmnow.tm_mon + 1U));
   setYear(static_cast<uint8_t>(tmnow.tm_year - 100U));
}

void DS3231::DS3231::setSecond(uint8_t second) {
   writeRegister(0x00, DS3231_Tools::decToBcd(second));
   uint8_t temp_buffer = readControlByte(1);
   writeControlByte((temp_buffer & 0b01111111), 1);
}

void DS3231::DS3231::setMinute(uint8_t minute) {
   writeRegister(0x01, DS3231_Tools::decToBcd(minute));
}

void DS3231::DS3231::setHour(uint8_t hour) {
   bool h12 = (readRegisterRaw(0x02) & 0b01000000);
   uint8_t temp_hour;

   if (h12) {
      bool am_pm = (hour > 11);
      temp_hour = hour;
      if (temp_hour > 11) {
         temp_hour = temp_hour - 12;
      }
      if (temp_hour == 0) {
         temp_hour = 12;
      }
      temp_hour = DS3231_Tools::decToBcd(temp_hour) | (am_pm << 5) | 0b01000000;
   } else {
      temp_hour = DS3231_Tools::decToBcd(hour) & 0b10111111;
   }
   writeRegister(0x02, temp_hour);
}

void DS3231::DS3231::setDoW(uint8_t dayOfWeek) {
   writeRegister(0x03, DS3231_Tools::decToBcd(dayOfWeek));
}

void DS3231::DS3231::setDate(uint8_t date) {
   writeRegister(0x04, DS3231_Tools::decToBcd(date));
}

void DS3231::DS3231::setMonth(uint8_t month) {
   writeRegister(0x05, DS3231_Tools::decToBcd(month));
}

void DS3231::DS3231::setYear(uint8_t year) {
   writeRegister(0x06, DS3231_Tools::decToBcd(year));
}

void DS3231::DS3231::set12hourMode() {
   uint8_t hour_byte = readRegisterRaw(0x02);
   bool currently_24h = !(hour_byte & 0b01000000);  // Bit 6 == 0 means 24h mode
   
   if (currently_24h) {
      // Convert from 24h to 12h
      uint8_t hour_24 = DS3231_Tools::bcdToDec(hour_byte & 0b00111111);
      uint8_t hour_12;
      bool is_pm = false;
      
      if (hour_24 == 0) {
         hour_12 = 12;  // 0:00 → 12:00 AM
         is_pm = false;
      } else if (hour_24 < 12) {
         hour_12 = hour_24;  // 1:00-11:00 → 1:00-11:00 AM
         is_pm = false;
      } else if (hour_24 == 12) {
         hour_12 = 12;  // 12:00 → 12:00 PM
         is_pm = true;
      } else {
         hour_12 = hour_24 - 12;  // 13:00-23:00 → 1:00-11:00 PM
         is_pm = true;
      }
      
      // Build new register value with 12h mode, PM flag, and new hour
      uint8_t new_hour = DS3231_Tools::decToBcd(hour_12);
      if (is_pm) {
         new_hour |= 0b01100000;  // Set Bit 6 (12h mode) and Bit 5 (PM)
      } else {
         new_hour |= 0b01000000;  // Set Bit 6 (12h mode) only
      }
      
      writeRegister(0x02, new_hour);
   }
   // If already in 12h mode, do nothing
}

void DS3231::DS3231::set24hourMode() {
   uint8_t hour_byte = readRegisterRaw(0x02);
   bool currently_12h = (hour_byte & 0b01000000);  // Bit 6 == 1 means 12h mode
   
   if (currently_12h) {
      // Convert from 12h to 24h
      bool is_pm = (hour_byte & 0b00100000);
      uint8_t hour_12 = DS3231_Tools::bcdToDec(hour_byte & 0b00011111);
      uint8_t hour_24;
      
      if (hour_12 == 12) {
         hour_24 = is_pm ? 12 : 0;  // 12:00 AM → 0:00, 12:00 PM → 12:00
      } else if (is_pm) {
         hour_24 = hour_12 + 12;  // 1:00-11:00 PM → 13:00-23:00
      } else {
         hour_24 = hour_12;  // 1:00-11:00 AM → 1:00-11:00
      }
      
      // Build new register value with 24h mode (Bit 6 = 0)
      uint8_t new_hour = DS3231_Tools::decToBcd(hour_24) & 0b10111111;  // Clear Bit 6
      
      writeRegister(0x02, new_hour);
   }
   // If already in 24h mode, do nothing
}

void DS3231::DS3231::begin() {
#if DS3231_RTC_HAS_WIRE
   _wire_adapter.begin();
#endif
   set24hourMode();
}

bool DS3231::DS3231::is24hourModeActive() {
   uint8_t hour_byte = readRegisterRaw(0x02);
   bool currently_12h = (hour_byte & 0b01000000);  // Bit 6 == 1 means 12h mode
   return !currently_12h;
}

float DS3231::DS3231::getTemperature() {
   uint8_t tMSB, tLSB;
   float temp3231;

   selectRegister(0x11);
   bus().requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 2);

   if(bus().available()) {
      tMSB = static_cast<uint8_t>(bus().read());
      tLSB = static_cast<uint8_t>(bus().read());

      int16_t itemp = static_cast<int16_t>(tMSB << 8 | (tLSB & 0xC0));
      temp3231 = ((float)itemp / 256.0f);
   }
   else {
      temp3231 = NAN;
   }
   return temp3231;
}

void DS3231::DS3231::getA1Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &Second, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM) {
   uint8_t temp_buffer;
   selectRegister(0x07);

   bus().requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 4);

   temp_buffer = static_cast<uint8_t>(bus().read());
   Second = DS3231_Tools::bcdToDec(temp_buffer & 0b01111111);
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>7;

   temp_buffer = static_cast<uint8_t>(bus().read());
   Minute = DS3231_Tools::bcdToDec(temp_buffer & 0b01111111);
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>6;

   temp_buffer = static_cast<uint8_t>(bus().read());
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>5;
   h12 = temp_buffer & 0b01000000;
   if (h12) {
      PM = temp_buffer & 0b00100000;
      Hour = DS3231_Tools::bcdToDec(temp_buffer & 0b00011111);
   } else {
      Hour = DS3231_Tools::bcdToDec(temp_buffer & 0b00111111);
   }

   temp_buffer = static_cast<uint8_t>(bus().read());
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>4;
   Dy = (temp_buffer & 0b01000000) >>6;
   if (Dy) {
      Day = DS3231_Tools::bcdToDec(temp_buffer & 0b00001111);
   } else {
      Day = DS3231_Tools::bcdToDec(temp_buffer & 0b00111111);
   }
}

void DS3231::DS3231::getA1Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &Second, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM, bool clearAlarmBits) {
   if (clearAlarmBits) {
      AlarmBits = 0x0;
   }
   getA1Time(Day, Hour, Minute, Second, AlarmBits, Dy, h12, PM);
}

void DS3231::DS3231::getA2Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM) {
   uint8_t temp_buffer;
   selectRegister(0x0b);

   bus().requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 3);
   temp_buffer = static_cast<uint8_t>(bus().read());
   Minute = DS3231_Tools::bcdToDec(temp_buffer & 0b01111111);
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000)>>3;

   temp_buffer = static_cast<uint8_t>(bus().read());
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000)>>2;
   h12 = temp_buffer & 0b01000000;
   if (h12) {
      PM = temp_buffer & 0b00100000;
      Hour = DS3231_Tools::bcdToDec(temp_buffer & 0b00011111);
   } else {
      Hour = DS3231_Tools::bcdToDec(temp_buffer & 0b00111111);
   }

   temp_buffer = static_cast<uint8_t>(bus().read());
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000)>>1;
   Dy = (temp_buffer & 0b01000000)>>6;
   if (Dy) {
      Day = DS3231_Tools::bcdToDec(temp_buffer & 0b00001111);
   } else {
      Day = DS3231_Tools::bcdToDec(temp_buffer & 0b00111111);
   }
}

void DS3231::DS3231::getA2Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM, bool clearAlarmBits) {
   if (clearAlarmBits) {
      AlarmBits = 0x0;
   }
   getA2Time(Day, Hour, Minute, AlarmBits, Dy, h12, PM);
}

void DS3231::DS3231::setA1Time(uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t Second, uint8_t AlarmBits, bool Dy, bool h12, bool PM) {
   uint8_t temp_buffer;
   bus().beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS);
   bus().write(0x07);
   bus().write(DS3231_Tools::decToBcd(Second) | ((AlarmBits & 0b00000001) << 7));
   bus().write(DS3231_Tools::decToBcd(Minute) | ((AlarmBits & 0b00000010) << 6));

   if (h12) {
      if (Hour > 12) {
         Hour = Hour - 12;
         PM = true;
      }
      if (PM) {
         temp_buffer = DS3231_Tools::decToBcd(Hour) | 0b01100000;
      } else {
         temp_buffer = DS3231_Tools::decToBcd(Hour) | 0b01000000;
      }
   } else {
      temp_buffer = DS3231_Tools::decToBcd(Hour);
   }
   temp_buffer = temp_buffer | ((AlarmBits & 0b00000100)<<5);
   bus().write(temp_buffer);

   temp_buffer = ((AlarmBits & 0b00001000)<<4) | DS3231_Tools::decToBcd(Day);
   if (Dy) {
      temp_buffer = temp_buffer | 0b01000000;
   }
   bus().write(temp_buffer);
   bus().endTransmission();
}

void DS3231::DS3231::setA2Time(uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t AlarmBits, bool Dy, bool h12, bool PM) {
   uint8_t temp_buffer;
   bus().beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS);
   bus().write(0x0b);
   bus().write(DS3231_Tools::decToBcd(Minute) | ((AlarmBits & 0b00010000) << 3));

   if (h12) {
      if (Hour > 12) {
         Hour = Hour - 12;
         PM = true;
      }
      if (PM) {
         temp_buffer = DS3231_Tools::decToBcd(Hour) | 0b01100000;
      } else {
         temp_buffer = DS3231_Tools::decToBcd(Hour) | 0b01000000;
      }
   } else {
      temp_buffer = DS3231_Tools::decToBcd(Hour);
   }
   temp_buffer = temp_buffer | ((AlarmBits & 0b00100000)<<2);
   bus().write(temp_buffer);

   temp_buffer = ((AlarmBits & 0b01000000)<<1) | DS3231_Tools::decToBcd(Day);
   if (Dy) {
      temp_buffer = temp_buffer | 0b01000000;
   }
   bus().write(temp_buffer);
   bus().endTransmission();
}

void DS3231::DS3231::turnOnAlarm(uint8_t alarmNumber) {
   uint8_t temp_buffer = readControlByte(0);
   if (alarmNumber == 1) {
      temp_buffer = temp_buffer | 0b00000101;
   } else {
      temp_buffer = temp_buffer | 0b00000110;
   }
   writeControlByte(temp_buffer, 0);
}

void DS3231::DS3231::turnOffAlarm(uint8_t alarmNumber) {
   uint8_t temp_buffer = readControlByte(0);
   if (alarmNumber == 1) {
      temp_buffer = temp_buffer & 0b11111110;
   } else {
      temp_buffer = temp_buffer & 0b11111101;
   }
   writeControlByte(temp_buffer, 0);
}

bool DS3231::DS3231::checkAlarmEnabled(uint8_t alarmNumber) {
   uint8_t result = 0x0;
   uint8_t temp_buffer = readControlByte(0);
   if (alarmNumber == 1) {
      result = temp_buffer & 0b00000001;
   } else {
      result = temp_buffer & 0b00000010;
   }
   return result;
}

bool DS3231::DS3231::checkIfAlarm(uint8_t alarmNumber, bool clearflag) {
   uint8_t result;
   uint8_t temp_buffer = readControlByte(1);
   if (alarmNumber == 1) {
      result = temp_buffer & 0b00000001;
      temp_buffer = temp_buffer & 0b11111110;
   } else {
      result = temp_buffer & 0b00000010;
      temp_buffer = temp_buffer & 0b11111101;
   }
   if (clearflag) {
      writeControlByte(temp_buffer, 1);
   }
   return result;
}

void DS3231::DS3231::enableOscillator(bool turnOn, bool onWithBattery, uint8_t frequency) {
   if (frequency > 3) {
      frequency = 3;
   }

   uint8_t temp_buffer = readControlByte(0) & 0b11100111;
   if (onWithBattery) {
      temp_buffer = temp_buffer | 0b01000000;
   } else {
      temp_buffer = temp_buffer & 0b10111111;
   }
   if (turnOn) {
      temp_buffer = temp_buffer & 0b01111011;
   } else {
      temp_buffer = temp_buffer | 0b10000000;
   }

   frequency = frequency << 3;
   temp_buffer = temp_buffer | frequency;
   writeControlByte(temp_buffer, 0);
}

void DS3231::DS3231::enable32kHz(bool activate32kHz) {
   uint8_t temp_buffer = readControlByte(1);
   if (activate32kHz) {
      temp_buffer = temp_buffer | 0b00001000;
   } else {
      temp_buffer = temp_buffer & 0b11110111;
   }
   writeControlByte(temp_buffer, 1);
}

bool DS3231::DS3231::oscillatorCheck() {
   uint8_t temp_buffer = readControlByte(1);
   bool result = true;
   if (temp_buffer & 0b10000000) {
      result = false;
   }
   return result;
}

// *****************************************
//   Private Functions of DS3231 object
// *****************************************

void DS3231::DS3231::selectRegister(uint8_t register_addr) {
   bus().beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS);
   bus().write(register_addr);
   bus().endTransmission();
}

uint8_t DS3231::DS3231::readRegisterRaw(uint8_t register_addr) {
   selectRegister(register_addr);
   bus().requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1);
   return static_cast<uint8_t>(bus().read());
}

void DS3231::DS3231::writeRegister(uint8_t register_addr, uint8_t value) {
   bus().beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS);
   bus().write(register_addr);
   bus().write(value);
   bus().endTransmission();
}

uint8_t DS3231::DS3231::readControlByte(bool which) {
   return readRegisterRaw(which ? 0x0f : 0x0e);
}

void DS3231::DS3231::writeControlByte(uint8_t control, bool which) {
   writeRegister(which ? 0x0f : 0x0e, control);
}
#pragma endregion DS3231