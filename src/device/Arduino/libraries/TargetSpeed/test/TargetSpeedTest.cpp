// (c) 2014 Julien Pilet <julien.pilet@gmail.com>

#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <device/Arduino/libraries/TargetSpeed/TargetSpeedTestData.h>

#include <gtest/gtest.h>

using namespace sail;

TEST(TargetSpeed, IreneSpeedUpWind) {
  TargetSpeedTable table;
  fillTestSpeedTable(&table);

  EXPECT_NEAR(1.0, getVmgSpeedRatio(table, 42, 10, 5.7), .1);
}

TEST(TargetSpeed, IreneSpeedDownWind) {
  TargetSpeedTable table;
  fillTestSpeedTable(&table);

  EXPECT_NEAR(1.0, getVmgSpeedRatio(table, 160, 10, 5.8), .1);
}
