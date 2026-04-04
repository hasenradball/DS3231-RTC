/**
 * @file    DS3231-RTC_Tools.h
 * @author  Frank Häfele
 * @date    31.03.2026
 * @version 1.2.0
 * @brief   Tools for the DS3231-RTC lib
 */

#pragma once

namespace DS3231_Tools {
   /**
    * @brief Convert value to binary coded decimal
    * 
    * @param value decimal value
    * @return uint8_t binary coded decimal value
    */
   inline uint8_t decToBcd(uint8_t value) {
      return ( (value / 10 * 16) + (value % 10) );
   }

   /**
    * @brief Convert binary coded decimal to decimal number
    * @param value binary coded decimal value
    * @return uint8_t decimal value
    */
   inline uint8_t bcdToDec(uint8_t value) {
      return ( (value / 16 * 10) + (value % 16) );
   }
}
