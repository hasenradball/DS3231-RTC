/**
 * @file    DS3231-RTC.cpp
 * @author  Frank Häfele
 * @date    31.03.2026
 * @version 1.2.0
 * @brief   Real-Time clock library based on Arduino Frameword
 */

#include "DS3231-RTC.h"
#include <math.h>

// These included for the DateTime class inclusion; will try to find a way to
// not need them in the future...
#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

// *****************************************
//  Static Functions only used in this file 
// *****************************************

static const uint8_t daysInMonth[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/**
 * @brief function which calculates if a year is a leap year
 * 
 * @param year 
 * @return true 
 * @return false 
 */
static bool isleapYear(const int16_t year) {
   // check if divisible by 4
   if(year % 4) {
      return false;
   }
   // only check other, when first failed
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

// Slightly modified from JeeLabs / Ladyada
// Get all date/time at once to avoid rollover (e.g., minute/second don't match)
// Commented to avoid compiler warnings, but keeping in case we want this
// eventually
// static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }
static uint8_t bcd2bin (uint8_t val) {
   return val - 6 * (val >> 4);
}


// *****************************************
//   Member functions for DateTime object
// *****************************************
DS3231::DateTime::DateTime (time_t unix_timestamp)
: _unix_timestamp{unix_timestamp}, _y2k_timestamp{unix_timestamp - UNIX_OFFSET}
{
   gmtime_r(&_unix_timestamp, &_tm);
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
   sscanf(date, "%s %2d %4d", month_buff, &day, &year);
   int month = (strstr(month_names, month_buff) - month_names) / 3 + 1;
   _tm.tm_year = year-1900;
   _tm.tm_mon = month-1;
   _tm.tm_mday = day;
   uint8_t hour, min, sec;
   sscanf(time, "%hhu:%hhu:%hhu", &hour, &min, &sec);
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

// *****************************************
//   Member functions for RTClib object
// *****************************************

DS3231::DateTime DS3231::RTClib::now(TwoWire &_Wire) {
   // This is the first register address (Seconds)
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0);
   // We'll read from here on for 7 uint8_ts from registers:
   // seconds, minutes, hours, day(1...7), date(1...31), month, year.
   _Wire.endTransmission();

   _Wire.requestFrom(CLOCK_ADDRESS, 7);
   int8_t sec = bcd2bin(_Wire.read() & 0x7F);
   int8_t min = bcd2bin(_Wire.read());
   int8_t hour = bcd2bin(_Wire.read());
   int8_t wday = bcd2bin(_Wire.read())-1;
   int8_t day = bcd2bin(_Wire.read());
   int8_t month = bcd2bin(_Wire.read());
   int16_t year = bcd2bin(_Wire.read()) + 2000;
   int16_t yday = calcYearDay(year, month, day);
   int16_t dst = -1;

   // REMARK: add DST calculation if needed, but therefore timezone info is needed!
   // use the complete set also yearday and dst for having a complete struct tm
   return DateTime{year, month, day, hour, min, sec, wday, yday, dst};
}

// *****************************************
//   Member functions for DS3231 object
// *****************************************

DS3231::DS3231::DS3231() : _Wire(Wire)
{}

DS3231::DS3231::DS3231(TwoWire &twowire) : _Wire(twowire)
{}

uint8_t DS3231::DS3231::getSecond() {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x00);
   _Wire.endTransmission();
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getMinute() {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x01);
   _Wire.endTransmission();
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getHour(bool& h12, bool& PM_time) {
   uint8_t temp_buffer;
   uint8_t hour;
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x02);
   _Wire.endTransmission();

   _Wire.requestFrom(CLOCK_ADDRESS, 1);
   temp_buffer = _Wire.read();
   h12 = temp_buffer & 0b01000000;
   if (h12) {
      PM_time = temp_buffer & 0b00100000;
      hour = bcdToDec(temp_buffer & 0b00011111);
   }
   else {
      hour = bcdToDec(temp_buffer & 0b00111111);
   }
   return hour;
}

uint8_t DS3231::DS3231::getDoW() {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x03);
   _Wire.endTransmission();
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getDate() {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x04);
   _Wire.endTransmission();
   return getRegisterValue();
}

uint8_t DS3231::DS3231::getMonth(bool &century) {
   uint8_t temp_buffer;
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x05);
   _Wire.endTransmission();

   _Wire.requestFrom(CLOCK_ADDRESS, 1);
   temp_buffer = _Wire.read();
   century = temp_buffer & 0b10000000;
   return (bcdToDec(temp_buffer & 0b01111111));
}

uint8_t DS3231::DS3231::getYear() {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x06);
   _Wire.endTransmission();
   return getRegisterValue();
}

void DS3231::DS3231::setEpoch(time_t epoch, bool flag_localtime) {
#if defined (__AVR__)
   epoch -= UNIX_OFFSET;
#endif
   struct tm tmnow;
   if (flag_localtime) {
      localtime_r(&epoch, &tmnow);
   }
   else {
      gmtime_r(&epoch, &tmnow);
   }
   setSecond(tmnow.tm_sec);
   setMinute(tmnow.tm_min);
   setHour(tmnow.tm_hour);
   setDoW(tmnow.tm_wday + 1U);
   setDate(tmnow.tm_mday);
   setMonth(tmnow.tm_mon + 1U);
   setYear(tmnow.tm_year - 100U);
}

void DS3231::DS3231::setSecond(uint8_t second) { 
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x00);
   _Wire.write(decToBcd(second));
   _Wire.endTransmission();
   // Clear OSF flag
   uint8_t temp_buffer = readControlByte(1);
   writeControlByte((temp_buffer & 0b01111111), 1);
}

void DS3231::DS3231::setMinute(uint8_t minute) {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x01);
   _Wire.write(decToBcd(minute));
   _Wire.endTransmission();
}

void DS3231::DS3231::setHour(uint8_t hour) {
   bool h12;
   uint8_t temp_hour;

   // Start by figuring out what the 12/24 mode is
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x02);
   _Wire.endTransmission();
   _Wire.requestFrom(CLOCK_ADDRESS, 1);
   h12 = (_Wire.read() & 0b01000000);
   // if h12 is true, it's 12h mode; false is 24h.

   if (h12) {
      // 12 hour
      bool am_pm = (hour > 11);
      temp_hour = hour;
      if (temp_hour > 11) {
         temp_hour = temp_hour - 12;
      }
      if (temp_hour == 0) {
         temp_hour = 12;
      }
      temp_hour = decToBcd(temp_hour) | (am_pm << 5) | 0b01000000;
   } else {
      // 24 hour
      temp_hour = decToBcd(hour) & 0b10111111;
   }

   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x02);
   _Wire.write(temp_hour);
   _Wire.endTransmission();
}

void DS3231::DS3231::setDoW(uint8_t dayOfWeek) {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x03);
   _Wire.write(decToBcd(dayOfWeek));
   _Wire.endTransmission();
}

void DS3231::DS3231::setDate(uint8_t date) {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x04);
   _Wire.write(decToBcd(date));
   _Wire.endTransmission();
}

void DS3231::DS3231::setMonth(uint8_t month) {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x05);
   _Wire.write(decToBcd(month));
   _Wire.endTransmission();
}

void DS3231::DS3231::setYear(uint8_t year) {
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x06);
   _Wire.write(decToBcd(year));
   _Wire.endTransmission();
}

/**
 * @brief sets the clock mode to .

* 
* @param h12 12h (true) or 24h (false)
*/
void DS3231::DS3231::setClockMode(bool h12) {
   // One thing that bothers me about how I've written this is that
   // if the read and right happen at the right hourly millisecond,
   // the clock will be set back an hour. Not sure how to do it better,
   // though, and as long as one doesn't set the mode frequently it's
   // a very minimal risk.
   // It's zero risk if you call this BEFORE setting the hour, since
   // the setHour() function doesn't change this mode.

   uint8_t temp_buffer;

   // Start by reading uint8_t 0x02.
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x02);
   _Wire.endTransmission();
   _Wire.requestFrom(CLOCK_ADDRESS, 1);
   temp_buffer = _Wire.read();

   // Set the flag to the requested value:
   if (h12) {
      temp_buffer = temp_buffer | 0b01000000;
   } else {
      temp_buffer = temp_buffer & 0b10111111;
   }

   // Write the uint8_t
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x02);
   _Wire.write(temp_buffer);
   _Wire.endTransmission();
}

float DS3231::DS3231::getTemperature() {
   uint8_t tMSB, tLSB;
   float temp3231;

   // temp registers (0x11...0x12) get updated automatically every 64 s
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x11);
   _Wire.endTransmission();
   _Wire.requestFrom(CLOCK_ADDRESS, 2);

   // Should I do more "if available" checks here?
   if(_Wire.available()) {
      // 2's complement int portion
      tMSB = _Wire.read();
      //fraction portion
      tLSB = _Wire.read(); 

      // Shift upper uint8_t, add lower
      int16_t  itemp  = ( tMSB << 8 | (tLSB & 0xC0) );
      // Scale and return
      temp3231 = ( (float)itemp / 256.0 );
   }
   else {
      // Impossible temperature => return NAN
      temp3231 = NAN; 
   }
   return temp3231;
}

void DS3231::DS3231::getA1Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &Second, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM) {
   uint8_t temp_buffer;
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x07);
   _Wire.endTransmission();

   _Wire.requestFrom(CLOCK_ADDRESS, 4);

   temp_buffer	= _Wire.read();	// Get A1M1 and A1 Seconds
   Second = bcdToDec(temp_buffer & 0b01111111);
   // put A1M1 bit in position 0 of DS3231_AlarmBits.
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>7;

   temp_buffer	= _Wire.read();	// Get A1M2 and A1 minutes
   Minute = bcdToDec(temp_buffer & 0b01111111);
   // put A1M2 bit in position 1 of DS3231_AlarmBits.
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>6;

   temp_buffer	= _Wire.read();	// Get A1M3 and A1 Hour
   // put A1M3 bit in position 2 of DS3231_AlarmBits.
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>5;
   // determine A1 12/24 mode
   h12 = temp_buffer & 0b01000000;
   if (h12) {
      PM = temp_buffer & 0b00100000;			// determine am/pm
      Hour = bcdToDec(temp_buffer & 0b00011111);	// 12-hour
   } else {
      Hour = bcdToDec(temp_buffer & 0b00111111);	// 24-hour
   }

   temp_buffer	= _Wire.read();	// Get A1M4 and A1 Day/Date
   // put A1M3 bit in position 3 of DS3231_AlarmBits.
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000) >>4;
   // determine A1 day or date flag
   Dy	= (temp_buffer & 0b01000000) >>6;
   if (Dy) {
      // alarm is by day of week, not date.
      Day = bcdToDec(temp_buffer & 0b00001111);
   } else {
      // alarm is by date, not day of week.
      Day = bcdToDec(temp_buffer & 0b00111111);
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
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x0b);
   _Wire.endTransmission();

   _Wire.requestFrom(CLOCK_ADDRESS, 3);
   temp_buffer	= _Wire.read();	// Get A2M2 and A2 Minutes
   Minute = bcdToDec(temp_buffer & 0b01111111);
   // put A2M2 bit in position 4 of DS3231_AlarmBits.
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000)>>3;

   temp_buffer	= _Wire.read();	// Get A2M3 and A2 Hour
   // put A2M3 bit in position 5 of DS3231_AlarmBits.
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000)>>2;
   // determine A2 12/24 mode
   h12	= temp_buffer & 0b01000000;
   if (h12) {
      PM = temp_buffer & 0b00100000;			// determine am/pm
      Hour = bcdToDec(temp_buffer & 0b00011111);	// 12-hour
   } else {
      Hour = bcdToDec(temp_buffer & 0b00111111);	// 24-hour
   }

   temp_buffer	= _Wire.read();	// Get A2M4 and A1 Day/Date
   // put A2M4 bit in position 6 of DS3231_AlarmBits.
   AlarmBits = AlarmBits | (temp_buffer & 0b10000000)>>1;
   // determine A2 day or date flag
   Dy = (temp_buffer & 0b01000000)>>6;
   if (Dy) {
      // alarm is by day of week, not date.
      Day	= bcdToDec(temp_buffer & 0b00001111);
   } else {
      // alarm is by date, not day of week.
      Day	= bcdToDec(temp_buffer & 0b00111111);
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
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x07);	// A1 starts at 07h
   // Send A1 second and A1M1
   _Wire.write(decToBcd(A1Second) | ((AlarmBits & 0b00000001) << 7));
   // Send A1 Minute and A1M2
   _Wire.write(decToBcd(A1Minute) | ((AlarmBits & 0b00000010) << 6));
   // Figure out A1 hour
   if (h12) {
      // Start by converting existing time to h12 if it was given in 24h.
      if (Hour > 12) {
         // well, then, this obviously isn't a h12 time, is it?
         Hour = Hour - 12;
         PM = true;
      }
      if (PM) {
         // Afternoon
         // Convert the hour to BCD and add appropriate flags.
         temp_buffer = decToBcd(A1Hour) | 0b01100000;
      } else {
         // Morning
         // Convert the hour to BCD and add appropriate flags.
         temp_buffer = decToBcd(A1Hour) | 0b01000000;
      }
   } else {
      // Now for 24h
      temp_buffer = decToBcd(A1Hour);
   }
   temp_buffer = temp_buffer | ((AlarmBits & 0b00000100)<<5);
   // A1 hour is figured out, send it
   _Wire.write(temp_buffer);
   // Figure out A1 day/date and A1M4
   temp_buffer = ((AlarmBits & 0b00001000)<<4) | decToBcd(Day);
   if (Dy) {
      // Set A1 Day/Date flag (Otherwise it's zero)
      temp_buffer = temp_buffer | 0b01000000;
   }
   _Wire.write(temp_buffer);
   // All done!
   _Wire.endTransmission();
}

void DS3231::setA2Time(uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t AlarmBits, bool Dy, bool h12, bool PM) {
   uint8_t temp_buffer;
   _Wire.beginTransmission(CLOCK_ADDRESS);
   _Wire.write(0x0b);	// A1 starts at 0bh
   // Send A2 Minute and A2M2
   _Wire.write(decToBcd(Minute) | ((AlarmBits & 0b00010000) << 3));
   // Figure out A2 hour
   if (h12) {
      // Start by converting existing time to h12 if it was given in 24h.
      if (Hour > 12) {
         // well, then, this obviously isn't a h12 time, is it?
         Hour = Hour - 12;
         PM = true;
      }
      if (PM) {
         // Afternoon
         // Convert the hour to BCD and add appropriate flags.
         temp_buffer = decToBcd(Hour) | 0b01100000;
      } else {
         // Morning
         // Convert the hour to BCD and add appropriate flags.
         temp_buffer = decToBcd(Hour) | 0b01000000;
      }
   } else {
      // Now for 24h
      temp_buffer = decToBcd(Hour);
   }
   // add in A2M3 bit
   temp_buffer = temp_buffer | ((AlarmBits & 0b00100000)<<2);
   // A2 hour is figured out, send it
   _Wire.write(temp_buffer);
   // Figure out A2 day/date and A2M4
   temp_buffer = ((AlarmBits & 0b01000000)<<1) | decToBcd(Day);
   if (Dy) {
      // Set A2 Day/Date flag (Otherwise it's zero)
      temp_buffer = temp_buffer | 0b01000000;
   }
   _Wire.write(temp_buffer);
   // All done!
   _Wire.endTransmission();
}

void DS3231::DS3231::turnOnAlarm(uint8_t alarmNumber) {
   uint8_t temp_buffer = readControlByte(0);
   // modify control uint8_t
   if (alarmNumber == 1) {
      temp_buffer = temp_buffer | 0b00000101;
   } else {
      temp_buffer = temp_buffer | 0b00000110;
   }
   writeControlByte(temp_buffer, 0);
}

void DS3231::DS3231::turnOffAlarm(uint8_t alarmNumber) {
   // Leaves interrupt pin alone.
   uint8_t temp_buffer = readControlByte(0);
   // modify control uint8_t
   if (alarmNumber == 1) {
      temp_buffer = temp_buffer & 0b11111110;
   } else {
      temp_buffer = temp_buffer & 0b11111101;
   }
   writeControlByte(temp_buffer, 0);
}

bool DS3231::DS3231::checkAlarmEnabled(uint8_t alarmNumber) {
   // Checks whether the given alarm is enabled.
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
      // Did alarm 1 go off?
      result = temp_buffer & 0b00000001;
      // clear flag
      temp_buffer = temp_buffer & 0b11111110;
   } else {
      // Did alarm 2 go off?
      result = temp_buffer & 0b00000010;
      // clear flag
      temp_buffer = temp_buffer & 0b11111101;
   }
   if (clearflag) {
      writeControlByte(temp_buffer, 1);
   }
   return result;
}

void DS3231::DS3231::enableOscillator(bool turnOn, bool onWithBattery, uint8_t frequency) {
   if (frequency > 3) frequency = 3;
   // read control uint8_t in, but zero out current state of RS2 and RS1.
   uint8_t temp_buffer = readControlByte(0) & 0b11100111;
   if (onWithBattery) {
      // turn on BBSQW flag
      temp_buffer = temp_buffer | 0b01000000;
   } else {
      // turn off BBSQW flag
      temp_buffer = temp_buffer & 0b10111111;
   }
   if (TF) {
      // set ~EOSC to 0 and INTCN to zero.
      temp_buffer = temp_buffer & 0b01111011;
   } else {
      // set ~EOSC to 1, leave INTCN as is.
      temp_buffer = temp_buffer | 0b10000000;
   }
   // shift frequency into bits 3 and 4 and set.
   frequency = frequency << 3;
   temp_buffer = temp_buffer | frequency;
   // And write the control bits
   writeControlByte(temp_buffer, 0);
}

void DS3231::DS3231::enable32kHz(bool activate32kHz) {
   uint8_t temp_buffer = readControlByte(1);
   if (32kHzON) {
      // turn on 32kHz pin
      temp_buffer = temp_buffer | 0b00001000;
   } else {
      // turn off 32kHz pin
      temp_buffer = temp_buffer & 0b11110111;
   }
   writeControlByte(temp_buffer, 1);
}

bool DS3231::DS3231::oscillatorCheck() {
   // Returns false if the oscillator has been off for some reason.
   // If this is the case, the time is probably not correct.
   uint8_t temp_buffer = readControlByte(1);
   bool result = true;
   if (temp_buffer & 0b10000000) {
      // Oscillator Stop Flag (OSF) is set, so return false.
      result = false;
   }
   return result;
}

// *****************************************
// 	 Private Functions of DS3231 object
// *****************************************

uint8_t DS3231::decToBcd(uint8_t val) {
// Convert normal decimal numbers to binary coded decimal
   return ( (val/10*16) + (val%10) );
}

uint8_t DS3231::bcdToDec(uint8_t val) {
// Convert binary coded decimal to normal decimal numbers
   return ( (val/16*10) + (val%16) );
}

uint8_t DS3231::readControlByte(bool which) {
   // Read selected control uint8_t
   // first uint8_t (0) is 0x0e, second (1) is 0x0f
   _Wire.beginTransmission(CLOCK_ADDRESS);
   if (which) {
      // second control uint8_t
      _Wire.write(0x0f);
   } else {
      // first control uint8_t
      _Wire.write(0x0e);
   }
   _Wire.endTransmission();
   _Wire.requestFrom(CLOCK_ADDRESS, 1);
   return _Wire.read();
}

void DS3231::writeControlByte(uint8_t control, bool which) {
   // Write the selected control uint8_t.
   // which=false -> 0x0e, true->0x0f.
   _Wire.beginTransmission(CLOCK_ADDRESS);
   if (which) {
      _Wire.write(0x0f);
   } else {
      _Wire.write(0x0e);
   }
   _Wire.write(control);
   _Wire.endTransmission();
}
