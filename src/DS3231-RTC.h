/**
 * @file    DS3231-RTC.h
 * @author  Frank Häfele
 * @date    31.03.2026
 * @version 1.2.0
 * @brief   Real-Time clock library based on Arduino Frameword
 */

#ifndef __DS3231_RTC_H__
#define __DS3231_RTC_H__

//#include <Arduino.h>
#include <time.h>
//#include <Wire.h>

namespace DS3231 {

   constexpr unsigned int CLOCK_ADDRESS {0x68};

   /**
    * @brief Seconds from 1/1/1970 to 1/1/2000.
    * AKA Difference between the Y2K and the UNIX epochs, in seconds.
    * To convert a Y2K timestamp to UNIX.
    * 
    */
   constexpr unsigned long UNIX_OFFSET {946684800UL};

   /**
    * @brief Seconds from 1/1/1990 to 1/1/2000.
    * AKA Difference between the Y2K and the NTP epochs, in seconds.
    * To convert a Y2K timestamp to NTP.
    * 
    */
   constexpr unsigned long  NTP_OFFSET {3155673600UL};

   class DateTime {
      public:
         /**
         * @brief Construct a new Date Time:: Date Time object
         * 
         * @param unix_timestamp for setup the date time members
         */
         DateTime (time_t unix_timestamp = 0);

         /**
         * @brief Construct a new Date Time object
         * 
         * @param year year YYYY as e.g. 2022
         * @param month months since January - [ 1...12 ]
         * @param mday day of the month - [ 1...31 ]
         * @param hour hours since midnight - [ 0...23 ]
         * @param min minutes after the hour - [ 0...59 ]
         * @param sec seconds after the minute - [ 0...59 ]
         * @param wday weekday since Sunday, Sunday is 1 - [ 1...7 ]
         * @param yday yearday since first january [0...365]
         * @param dst daylight saving time / summer time
         */
         DateTime (int16_t year, int8_t month, int8_t mday,
                  int8_t hour = 0, int8_t min = 0, int8_t sec = 0,
                  int8_t wday = 0, int16_t yday = 0, int16_t dst = -1);

         /**
         * @brief Construct a new Date Time:: Date Time object by givin the precompiler marcos
         * as __DATE__ and __TIME__
         * 
         * @param date as  Mmm dd yyyy (e.g. "Jan 14 2012")
         * @param time as HH:MM:SS (e.g. "23:59:01")
         */
         DateTime (const char *date, const char *time);
         
         /**
         * @brief Get the Year value
         * 
         * @return int16_t year as YYYY e.g. 2022
         */
         inline int16_t getYear() const { return _tm.tm_year + 1900; }

         /**
         * @brief Get the Month value
         * 
         * @return int8_t month 1...12
         */
         int8_t getMonth() const { return _tm.tm_mon + 1; }

         /**
         * @brief Get the Day value
         * 
         * @return int8_t day 1...31
         */
         int8_t getDay() const { return _tm.tm_mday; }

         /**
         * @brief Get the Hour value
         * 
         * @return int8_t hour as 0...23 
         */
         int8_t getHour() const { return _tm.tm_hour; }

         /**
         * @brief Get the Minute value
         * 
         * @return int8_t minute as 0...59
         */
         int8_t getMinute()  const   { return _tm.tm_min; }

         /**
         * @brief Get the Second value
         * 
         * @return int8_t second as 0...60
         */
         int8_t getSecond() const { return _tm.tm_sec; }

         /**
         * @brief Get the Week Day value
         * 
         * @return int8_t days since sunday
         */
         int8_t getWeekDay() const { return _tm.tm_wday; }

         /**
         * @brief Get the Year Day value
         * 
         * @return int16_t days since January 1 as 0...365
         */
         int16_t getYearDay() const	{ return _tm.tm_yday; }

         /**
         * @brief get daylight saving time
         * 
         * @retval >0: DST is active
         * @retval  0: DST is not active
         * @retval <0: no info avalilable
         */
         int16_t getDST()    const   { return _tm.tm_isdst; }

         /**
         * @brief function to format a DateTime string in an buffer based on the standard strftime function
         * 
         *  see: https://cplusplus.com/reference/ctime/strftime/ 
         *  or:  https://en.cppreference.com/w/cpp/chrono/c/strftime
         * 
         * @param buffer buffer for time string 
         * @param buffersize size of buffer
         * @param formatSpec define format see strftime
         * @return size_t length of used buffer
         */
         size_t strf_DateTime(char *buffer, size_t buffersize, const char *formatSpec = "%a %h %d %T %Y");

         /**
         * @brief time_t value as seconds from 1/1/2000 to now. 
         * Difference between the Y2K and the UNIX epochs, in seconds
         * 
         * @return time_t seconds from 1/1/2000 to now
         * 
         */
         time_t getY2kTime() const { return _y2k_timestamp; }

         /**
         * @brief Get the Unix Time value. AKA EPOCH.
         * THE ABOVE COMMENT IS CORRECT FOR LOCAL TIME; TO USE THIS COMMAND TO
         * OBTAIN TRUE UNIX TIME SINCE EPOCH, YOU MUST CALL THIS COMMAND AFTER
         * SETTING YOUR CLOCK TO UTC
         * 
         * @return time_t epoch since 1/1/1970
         */
         time_t getUnixTime() const  { return _unix_timestamp; }
      
      private:
         /**
         * @brief Set the timestamps for _y2k_timestamp and _unix_timestamp by struct tm entries
         * 
         */
         void set_timstamps();

      protected:
         /**
         * @brief internal unix timestamp from 1/1/1970 to now
         * 
         */
         time_t _unix_timestamp;

         /**
         * @brief internal y2k timestamp from 1/1/2000 to now
         * 
         */
         time_t _y2k_timestamp;

         /**
         * @brief internal struct tm
         * 
         */
         struct tm _tm;
   };

   class RTClib {
      public:
         /**
            * @brief get the actual timestamp snapshot
            * 
            * @param _Wire 
            * @return DateTime 
            */
         static DateTime now(TwoWire &_Wire = Wire);
   };

   class DS3231 {
      public:
         /**
         * @brief Construct a new DS3231::DS3231 object
         * initialize the internal _Wire with the Wire object
         */
         DS3231();

         /**
         * @brief Construct a new DS3231::DS3231 object
         * 
         * @param twowire reference of TwoWire object
         */
         DS3231(TwoWire &twowire);

         // ************************************
         //      Time-retrieval functions
         // ************************************
         /**
         * @brief Get the second of the DS3231 module
         * 
         * @return uint8_t 0...59
         */
         uint8_t getSecond();

         /**
         * @brief Get the minute of the DS3231 module
         * 
         * @return uint8_t 0...59
         */
         uint8_t getMinute();

         // Get the hour of the DS3231 module, 
         /**
         * @brief Get the hour of the DS3231 module. in addition, this function
         * returns the values of the 12/24-hour flag and the AM/PM flag.
         * 
         * @param h12 reference of h12 - true when 12 h mode
         * @param PM_time reference of pm time - true when pm
         * @return uint8_t 1...12 / 0...23
         */
         uint8_t getHour(bool& h12, bool& PM_time);

         /**
         * @brief Get the DayOfWeek of the DS3231 module
         * 
         * @return uint8_t 1...7
         */
         uint8_t getDoW();

         /**
         * @brief Get the date of the DS3231 module
         * 
         * @return uint8_t 1...31
         */
         uint8_t getDate();

         /**
         * @brief Get the month and the century roll over bit of the DS3231 module
         * 
         * @param century reference of century bit; toggles when value changes from 99 -> 00
         * @return uint8_t value of month 1...12
         */
         uint8_t getMonth(bool &century);

         /**
         * @brief Get the Year of the DS3231 module
         * 
         * @return uint8_t 0...99
         */
         uint8_t getYear();


         // ************************************
         //        Time-setting functions
         // ************************************
         // Note that none of these check for sensibility: You can set the
         // date to July 42nd and strange things will probably result.

         // set epoch function gives the epoch as parameter and feeds the RTC
         // epoch = UnixTime and starts at 01.01.1970 00:00:00

         /**
          * @brief Set the DS3231 by giving the Unix Epoch.
          * Seconds since January 1st 1970 00:00:00. 
          * HINT: => the AVR time.h lib is based on the year 2000
          * 
          * @param epoch seconds since 01.01.1970 00:00:00
          * @param flag_localtime true if epoch represents local timestamp false otherwise
          */
         void setEpoch(time_t epoch = 0, bool flag_localtime = false);

         /**
         * @brief Set the second of the DS3231 module
         * This function also resets the Oscillator Stop Flag, which is set
         * whenever power is interrupted.
         * @param second 0...59
         */
         void setSecond(uint8_t second);

         /**
          * @brief Set the Minute of the DS3231 module
          * 
          * @param minute 0...59
          */
         void setMinute(uint8_t minute);
         
         
         /**
          * @brief Sets the hour, without changing 12/24h mode.
          * The hour must be in 24h format.
          * 
          * @param hour 0...23
          */
         void setHour(uint8_t hour);


         /**
          * @brief Sets the Day of Week of the DS3231 module
          * 
          * @param dayOfWeek 1...7
          */
         void setDoW(uint8_t dayOfWeek);

         /**
          * @brief Sets the Date/Day of the DS3231 module
          * 
          * @param date 1...31
          */
         void setDate(uint8_t date);
         
         // Sets the Month of the DS3231 module
         /**
          * @brief Sets the Month of the DS3231 module
          * 
          * @param month 1...12
          */
         void setMonth(uint8_t month);

         /**
          * @brief Sets the Year of the DS3231 module.
          * 
          * @param year 0...99
          */
         void setYear(uint8_t year);

         /**
          * @brief Sets the Hour format (12h/24h) of the DS3231 module
          * 
          * @param h12 true/high for 12 h format, false for 24 h
          */
         void setClockMode(bool h12 = false);


         // ************************************
         //        Temperature Getter function
         // ************************************
         // get temperature of the DS3231 module
         /**
          * @brief read the internal temperature sensor of the DS3231 module
          * 
          * @return float temperature measured in DS3231 module
          */
         float getTemperature();

         // ************************************
         //        Alarm Getter functions
         // ************************************
         /* Retrieves everything you could want to know about alarm
          * one.
          * Dy true makes the alarm go on Day = Day of Week,
          * Dy false makes the alarm go on Day = Date of month.
          *
          * uint8_t AlarmBits sets the behavior of the alarms:
          *	Dy	A1M4	A1M3	A1M2	A1M1	Rate
          *	X	1		1		1		1		Once per second
          *	X	1		1		1		0		Alarm when seconds match
          *	X	1		1		0		0		Alarm when min, sec match
          *	X	1		0		0		0		Alarm when hour, min, sec match
          *	0	0		0		0		0		Alarm when date, h, m, s match
          *	1	0		0		0		0		Alarm when DoW, h, m, s match
          *
          *	Dy	A2M4	A2M3	A2M2	Rate
          *	X	1		1		1		Once per minute (at seconds = 00)
          *	X	1		1		0		Alarm when minutes match
          *	X	1		0		0		Alarm when hours and minutes match
          *	0	0		0		0		Alarm when date, hour, min match
          *	1	0		0		0		Alarm when DoW, hour, min match
          *
          *	Note: uint8_t AlarmBits is not explicitly cleared for the getAXTime methods to
          *	support sequential retrieval of both alarms with the same uint8_t AlarmBits.
          *	Use the flag bool clearAlarmBits=True to explicitly clear uint8_t AlarmBits on
          *  call to getAXTime.
          */
         
         /**
          * @brief get the time of alarm1
          * 
          * @param Day 
          * @param Hour 
          * @param Minute 
          * @param Second 
          * @param AlarmBits 
          * @param Dy 
          * @param h12 
          * @param PM 
          */
         void getA1Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &Second, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM);

         /**
          * @brief getA1Time();, but A2 only goes on seconds == 00.
          * 
          * @param Day 
          * @param Hour 
          * @param Minute 
          * @param AlarmBits 
          * @param Dy 
          * @param h12 
          * @param PM 
          */
         void getA2Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM);
         
         /**
          * @brief Same as getA1Time();, but clears uint8_t AlarmBits.
          * 
          * @param Day 
          * @param Hour 
          * @param Minute 
          * @param Second 
          * @param AlarmBits 
          * @param Dy 
          * @param h12 
          * @param PM 
          * @param clearAlarmBits 
          */
         void getA1Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &Second, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM, bool clearAlarmBits);

         

         /**
          * @brief Same as getA1Time();, but clears uint8_t AlarmBits.
          * 
          * @param Day 
          * @param Hour 
          * @param Minute 
          * @param AlarmBits 
          * @param Dy 
          * @param h12 
          * @param PM 
          * @param clearAlarmBits 
          */
         void getA2Time(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &AlarmBits, bool &Dy, bool &h12, bool &PM, bool clearAlarmBits);
         
         /**
          * @brief  Set the details for Alarm 1
          * 
          * @param Day 
          * @param Hour 
          * @param Minute 
          * @param Second 
          * @param AlarmBits 
          * @param Dy 
          * @param h12 
          * @param PM 
          */
         void setA1Time(uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t Second, uint8_t AlarmBits, bool Dy, bool h12, bool PM);
         
         /**
          * @brief Set the details for Alarm 2
          * 
          * @param Day 
          * @param Hour 
          * @param Minute 
          * @param AlarmBits 
          * @param Dy 
          * @param h12 
          * @param PM 
          */
         void setA2Time(uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t AlarmBits, bool Dy, bool h12, bool PM);
         
         /**
          * @brief Enables alarm 1 or 2 and the external interrupt pin.
          * If Alarm != 1, it assumes Alarm == 2.
          * 
          * @param alarmNumber set 1 or 2
          */
         void turnOnAlarm(uint8_t alarmNumber);

         /**
          * @brief Disables alarm 1 or 2 (default is 2 if Alarm != 1),
         // and leaves the interrupt pin alone.
          * 
          * @param alarmNumber 
          */
         void turnOffAlarm(uint8_t alarmNumber);

         /**
          * @brief checks if alarm is enabled
          * 
          * @param alarmNumber 1 or 2 
          * @return true if alarm is enabled
          * @return false if alarm is disabled
          */
         bool checkAlarmEnabled(uint8_t alarmNumber);
         
         // Checks whether the indicated alarm (1 or 2, 2 default);
         // has been activated. IF clearflag is set, clears alarm flag.
         /**
          * @brief check if alarm has triggered
          * 
          * @param alarmNumber 1 or 2
          * @param clearflag clears the alarm flag (default)
          * @return true alarm has triggered
          * @return false has not triggered 
          */
         bool checkIfAlarm(uint8_t alarmNumber, bool clearflag = true);
            

         // ************************************
         //        Oscillator functions
         // ************************************
         // turns oscillator on or off. True is on, false is off.
         // if battery is true, turns on even for battery-only operation,
         // otherwise turns off if Vcc is off.
         // frequency must be 0, 1, 2, or 3.
         // 0 = 1 Hz
         // 1 = 1.024 kHz
         // 2 = 4.096 kHz
         // 3 = 8.192 kHz (Default if frequency uint8_t is out of range);

         /**
          * @brief Turns oscillator ON or OFF.
          * If battery is true, turns on even for battery-only operation,
          * otherwise turns off if Vcc is off.
          * frequency must be 0, 1, 2, or 3.
          * 0 = 1 Hz
          * 1 = 1.024 kHz
          * 2 = 4.096 kHz
          * 3 = 8.192 kHz (Default if frequency uint8_t is out of range);
          * 
          * @param turnOn true to turn on false otherwise 
          * @param onWithBattery true for battery operation also
          * @param frequency set 0, 1, 2 or 3 to select frequency
          */
         void enableOscillator(bool turnOn, bool onWithBattery, uint8_t frequency);

         /**
          * @brief Switch ON the 32kHz output pin (true); or off (false)
          * 
          * @param activate32kHz 
          */
         void enable32kHz(bool activate32kHz);
         
         /**
          * @brief Checks the status of the Oscillator Stop Flag (OSF).
          * If this returns false, then the clock is probably not
          * giving you the correct time.
          * The OSF is cleared by function setSecond().
          * 
          * @return true if oscillator is running.
          * @return false if oscillator has stopped
          */
         bool oscillatorCheck();


      private:
         /**
         * @brief internal Twowire object reference
         * 
         */
         TwoWire &_Wire;
         
         // the getter functions retrieve current values of the registers.
         uint8_t getRegisterValue() {
            _Wire.requestFrom(CLOCK_ADDRESS, 1);
            return bcdToDec(_Wire.read());
         }

         // Convert normal decimal numbers to binary coded decimal
         uint8_t decToBcd(uint8_t val);
         // Convert binary coded decimal to normal decimal numbers
         uint8_t bcdToDec(uint8_t val);


      protected:
         // Read selected control uint8_t: (0); reads 0x0e, (1) reads 0x0f
         uint8_t readControlByte(bool which);
         
         // Write the selected control uint8_t.
         // which == false -> 0x0e, true->0x0f.
         void writeControlByte(uint8_t control, bool which);
   };
}
#endif
