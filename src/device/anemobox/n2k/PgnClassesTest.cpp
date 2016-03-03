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
  EXPECT_EQ(windData.reference().get(),  PgnClasses::WindData::Reference::Apparent);
}

TEST(PgnClassesTest, WindDataNotAvailable) {
  uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  PgnClasses::WindData windData(data, 8);

  EXPECT_FALSE(windData.valid());
  EXPECT_FALSE(windData.reference().defined());
}

class TestWindVisitor : public PgnClasses::PgnVisitor {
  protected:
    bool apply(const PgnClasses::CanPacket& c, const PgnClasses::WindData& packet) override {
      if (!packet.valid()) {
        return false;
      }
      if (c.longSrc != "MyWindsensor") {
        return false;
      }

      EXPECT_TRUE(packet.windSpeed().defined());
      EXPECT_NEAR(packet.windSpeed().get().metersPerSecond(), 0.25, 0.01);
      EXPECT_TRUE(packet.windAngle().defined());
      EXPECT_NEAR(packet.windAngle().get().radians(), 3.0892, 0.0001);
      return true;
    }
};

TEST(PgnClassesTest, WindVisitor) {
  std::vector<uint8_t> data{0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA, 0xFF, 0xFF};
  TestWindVisitor visitor;
  uint8_t shortSrc = 119;
  EXPECT_TRUE(visitor.visit(PgnClasses::CanPacket{
    "MyWindsensor", shortSrc, PgnClasses::WindData::ThisPgn, data}));
}

