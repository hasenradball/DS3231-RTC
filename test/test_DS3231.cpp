#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <time.h>
#include "DS3231-RTC.h"
using ::testing::Return;

TEST(DS3231Tools_BCD, BinaryDecodedDecimalToDecimal_0) {
   EXPECT_EQ(DS3231_Tools::bcdToDec(0x0), 0U);
}

TEST(DS3231Tools_BCD, BinaryDecodedDecimalToDecimal_9) {
   EXPECT_EQ(DS3231_Tools::bcdToDec(0x9), 9U);
}

TEST(DS3231Tools_BCD, BinaryDecodedDecimalToDecimal_10) {
   EXPECT_EQ(DS3231_Tools::bcdToDec(0x10), 10U);
}

TEST(DS3231Tools_BCD, BinaryDecodedDecimalToDecimal_59) {
   EXPECT_EQ(DS3231_Tools::bcdToDec(0x59), 59U);
}

TEST(DS3231Tools_BCD, DecimalToBinaryDecodedDecimal_0) {
   EXPECT_EQ(DS3231_Tools::decToBcd(0U), 0x0);
}

TEST(DS3231Tools_BCD, DecimalToBinaryDecodedDecimal_9) {
   EXPECT_EQ(DS3231_Tools::decToBcd(9U), 0x9);
}

TEST(DS3231Tools_BCD, DecimalToBinaryDecodedDecimal_10) {
   EXPECT_EQ(DS3231_Tools::decToBcd(10U), 0x10);
}

TEST(DS3231Tools_BCD, DecimalToBinaryDecodedDecimal_59) {
   EXPECT_EQ(DS3231_Tools::decToBcd(59U), 0x59);
}

TEST(DS3231Tools_leapYear, IsLeapYear_2023) {
   EXPECT_FALSE(DS3231_Tools::isleapYear(2023));
}

TEST(DS3231Tools_leapYear, IsLeapYear_2024) {
   EXPECT_TRUE(DS3231_Tools::isleapYear(2024));
}

TEST(DS3231Tools_leapYear, IsLeapYear_2032) {
   EXPECT_TRUE(DS3231_Tools::isleapYear(2032));
}

TEST(DS3231Tools_leapYear, IsLeapYear_1700) {
   EXPECT_FALSE(DS3231_Tools::isleapYear(1700));
}

TEST(DS3231Tools_leapYear, IsLeapYear_1800) {
   EXPECT_FALSE(DS3231_Tools::isleapYear(1800));
}

TEST(DS3231Tools_leapYear, IsLeapYear_1900) {
   EXPECT_FALSE(DS3231_Tools::isleapYear(1900));
}

TEST(DS3231Tools_leapYear, IsLeapYear_2000) {
   EXPECT_TRUE(DS3231_Tools::isleapYear(2000));
}

class MockBus : public DS3231::BusInterface {
   public:
      MOCK_METHOD(void, beginTransmission, (uint8_t address), (override));
      MOCK_METHOD(size_t, write, (uint8_t value), (override));
      MOCK_METHOD(uint8_t, endTransmission, (), (override));
      MOCK_METHOD(uint8_t, requestFrom, (uint8_t address, uint8_t quantity), (override));
      MOCK_METHOD(int, read, (), (override));
      MOCK_METHOD(int, available, (), (override));
};

class DS3231MockTest : public ::testing::Test {
   protected:
      MockBus bus;
      DS3231::DS3231 rtc{bus};
};

TEST_F(DS3231MockTest, GetSecondReadsSecondRegister) {
   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x00)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   EXPECT_CALL(bus, read()).WillOnce(Return(0x59));
   // DS3231 holds value in binary decoded decimal here 0x59 for 59 decimal
   EXPECT_EQ(rtc.getSecond(), 59);
}

TEST_F(DS3231MockTest, GetMinuteReadsMinuteRegister) {
   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x01)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   EXPECT_CALL(bus, read()).WillOnce(Return(0x42));

   EXPECT_EQ(rtc.getMinute(), 42);
}

TEST_F(DS3231MockTest, GetHourReadsFlagsAndBcdValue) {
   bool h12 = false;
   bool pm = false;

   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x02)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   EXPECT_CALL(bus, read()).WillOnce(Return(0x63));

   EXPECT_EQ(rtc.getHour(h12, pm), 3U);
   EXPECT_TRUE(h12);
   EXPECT_TRUE(pm);
}

TEST_F(DS3231MockTest, SetHourWritesUpdatedHourRegister) {
   ::testing::InSequence sequence;

   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x02)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   EXPECT_CALL(bus, read()).WillOnce(Return(0x00));

   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x02)).WillOnce(Return(1));
   EXPECT_CALL(bus, write(0x23)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));

   rtc.setHour(23);
}

TEST_F(DS3231MockTest, GetDoWReadsDoWRegister) {
   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x03)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   EXPECT_CALL(bus, read()).WillOnce(Return(0x03));

   EXPECT_EQ(rtc.getDoW(), 3U);
}

TEST_F(DS3231MockTest, GetDayReadsDateRegister) {
   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x04)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   EXPECT_CALL(bus, read()).WillOnce(Return(0x14));

   EXPECT_EQ(rtc.getDate(), 14U);
}

TEST_F(DS3231MockTest, GetMonthReadsMonthRegister) {
   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x05)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   EXPECT_CALL(bus, read()).WillOnce(Return(0x12));

   bool century;
   EXPECT_EQ(rtc.getMonth(century), 12U);
}

TEST_F(DS3231MockTest, GetYearReadsYearRegister) {
   EXPECT_CALL(bus, beginTransmission(DS3231_Constants::DS3231_I2C_ADDRESS));
   EXPECT_CALL(bus, write(0x06)).WillOnce(Return(1));
   EXPECT_CALL(bus, endTransmission()).WillOnce(Return(0));
   EXPECT_CALL(bus, requestFrom(DS3231_Constants::DS3231_I2C_ADDRESS, 1)).WillOnce(Return(1));
   // write Year 22 (bcd 0x22 => dec 22)
   EXPECT_CALL(bus, read()).WillOnce(Return(0x22));

   EXPECT_EQ(rtc.getYear(), 22U);
}