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
  BitStream stream(data, 1);

  PgnClasses::WindData x(&stream);
  EXPECT_FALSE(x.valid());
}

TEST(PgnClassesTest, WindData) {
  uint8_t data[] = {0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA, 0xFF, 0xFF};
  BitStream stream(data, 8);

  PgnClasses::WindData windData(&stream);
  EXPECT_TRUE(windData.valid());

}
