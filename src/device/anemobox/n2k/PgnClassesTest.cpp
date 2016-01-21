/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <device/anemobox/n2k/PgnClasses.h>
#include <device/anemobox/n2k/BitStream.h>

TEST(PgnClassesTest, DefaultWindData) {
  PgnClasses::WindData x;
  EXPECT_FALSE(x.valid());
}

TEST(PgnClassesTest, InvalidWindData) {
  uint8_t data[] = {0xFF};

  PgnClasses::WindData x(data, 1);
  EXPECT_FALSE(x.valid());
}

TEST(PgnClassesTest, WindData) {
  uint8_t data[] = {0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA, 0xFF, 0xFF};

  PgnClasses::WindData windData(data, 8);
  EXPECT_TRUE(windData.valid());

  EXPECT_NEAR(windData.windSpeed().get().metersPerSecond(), 0.25, 0.01);
  EXPECT_NEAR(windData.windAngle().get().radians(), 3.0892, 0.0001);
  EXPECT_EQ(windData.reference().get(), 2);
}

TEST(PgnClassesTest, WindDataNotAvailable) {
  uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  PgnClasses::WindData windData(data, 8);
  EXPECT_TRUE(windData.valid());

  EXPECT_FALSE(windData.windSpeed().defined());
  EXPECT_FALSE(windData.windAngle().defined());
  EXPECT_FALSE(windData.reference().defined());
}
