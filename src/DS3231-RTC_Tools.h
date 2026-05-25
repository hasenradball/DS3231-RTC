/**
 * @file    DS3231-RTC_Tools.h
 * @author  Frank Häfele
 * @date    31.03.2026
 * @version 1.2.0
 * @brief   Tools for the DS3231-RTC lib
 */

#pragma once
#include "DS3231-RTC_Constants.h"

namespace DS3231_Tools {
   /**
    * @brief Convert value to binary coded decimal
    *
    * @param value decimal value
    * @return uint8_t binary coded decimal value
    */
   constexpr inline uint8_t decToBcd(uint8_t value) {
      return value + 6 * (value / 10);
   }

   /**
    * @brief Convert binary coded decimal to decimal number
    * @param value binary coded decimal value
    * @return uint8_t decimal value
    */
   constexpr inline uint8_t bcdToDec(uint8_t value) {
      return value - 6 * (value >> 4);
   }

   /**
    * @brief function which calculates if a year is a leap year
    *
    * @param year
    * @return true
    * @return false
    */
   constexpr inline bool isleapYear(const int16_t year) {
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
   constexpr int16_t calcYearDay(const int16_t year, const int8_t month,
                              const int8_t day) {
      uint16_t days = day - 1;
      for (uint8_t i = 1; i < month; ++i) {
         days += DS3231_Constants::daysInMonth[i-1];
      }
      if (month > 2 && isleapYear(year)) {
         ++days;
      }
      return days;
   }
} // namespace DS3231_Tools
