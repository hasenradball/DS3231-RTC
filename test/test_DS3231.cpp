#include <gtest/gtest.h>
#include "DS3231-RTC.h"


class DS3231Test : public ::testing::Test {
   protected:
      DS3231::DS3231 *ds3231;

      void SetUp() override {
         // wird vor jedem Test ausgeführt
         ds3231 = new DS3231::DS3231();
      }

      void TearDown() override {
         // wird nach jedem Test ausgeführt
         delete ds3231;
      }
};

// =====================================================
//  check begin of epoch
// =====================================================

TEST_F(DS3231Test, CheckBeginOfEpoch) {
  ds3231->setEpoch(0);

  EXPECT_EQ(ds3231->getYear(), 1970);
}