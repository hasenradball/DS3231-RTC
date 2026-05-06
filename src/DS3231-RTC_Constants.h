/**
 * @file    DS3231-RTC_Constants.h
 * @author  Frank Häfele
 * @date    31.03.2026
 * @version 1.2.0
 * @brief   Constants for the DS3231-RTC lib
 */

#pragma once

namespace DS3231_Constants {
   /**
    * @brief I2C Address of the DS3231 Module
    * 
    */
   constexpr unsigned int DS3231_I2C_ADDRESS {0x68};

   /**
    * @brief Seconds from 1/1/1970 to 1/1/2000.
    * AKA Difference between the Y2K and the UNIX epochs, in seconds.
    * To convert a Y2K timestamp to UNIX.
    * 
    */
   //constexpr unsigned long UNIX_OFFSET {946684800UL};

   /**
    * @brief Seconds from 1/1/1990 to 1/1/2000.
    * AKA Difference between the Y2K and the NTP epochs, in seconds.
    * To convert a Y2K timestamp to NTP.
    * 
    */
   //constexpr unsigned long  NTP_OFFSET {3155673600UL};
}