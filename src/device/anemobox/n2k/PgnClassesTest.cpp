/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <device/anemobox/n2k/PgnClasses.h>
#include <device/anemobox/n2k/BitStream.h>

using namespace PgnClasses;

template <typename T>
T recode(const T& x) {
  auto data = x.encode();
  return T(data.data(), data.size());
}


TEST(PgnClassesTest, DefaultWindData) {
  PgnClasses::WindData x;
  EXPECT_FALSE(x.valid());
  EXPECT_FALSE(recode(x).valid());
}

TEST(PgnClassesTest, InvalidWindData) {
  uint8_t data[] = {0xFF};

  PgnClasses::WindData x(data, 1);
  EXPECT_FALSE(x.valid());
  EXPECT_FALSE(recode(x).valid());
}

void testWindData(const WindData& windData) {
  EXPECT_TRUE(windData.valid());
  EXPECT_NEAR(windData.windSpeed().get().metersPerSecond(), 0.25, 0.01);
  EXPECT_NEAR(windData.windAngle().get().radians(), 3.0892, 0.0001);
  EXPECT_EQ(windData.reference().get(),  PgnClasses::WindData::Reference::Apparent);
}

TEST(PgnClassesTest, WindData) {
  uint8_t data[] = {0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA, 0xFF, 0xFF};

  PgnClasses::WindData windData(data, 8);
  testWindData(windData);
  testWindData(recode(windData));
}

void testWindDataNotAvailable(const WindData& windData) {
  EXPECT_FALSE(windData.valid());
  EXPECT_FALSE(windData.reference().defined());
}

TEST(PgnClassesTest, WindDataNotAvailable) {
  uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  PgnClasses::WindData windData(data, 8);
  testWindDataNotAvailable(windData);
  testWindDataNotAvailable(recode(windData));
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

void testPositionRapidUpdate(const PositionRapidUpdate& pru) {
  EXPECT_TRUE(pru.latitude().defined());
  EXPECT_TRUE(pru.longitude().defined());
  EXPECT_NEAR(41.3797185, pru.latitude().get().degrees(), 1e-7);
  EXPECT_NEAR(2.1857485, pru.longitude().get().degrees(), 1e-7);
}

TEST(PgnClassesTest, PositionRapidUpdate) {
  uint8_t data[] = { 0x41, 0x0b, 0xaa, 0x18, 0xcd, 0x84, 0x4d, 0x01 };

  PositionRapidUpdate pru(data, sizeof(data));
  testPositionRapidUpdate(pru);
  testPositionRapidUpdate(recode(pru));
}

TEST(PgnClassesTest, GnssPositionData) {
  uint8_t data[] = {
    // Commented out bytes are the header bytes used by the fastpacket
    // system.
    /* 0x20, 0x2F,*/ 0x3E, 0xA7, 0x42, 0x60, 0x39, 0xEA,
    /* 0x21,*/ 0x12, 0x80, 0xA7, 0xAA, 0x1E, 0x67, 0x1A,
    /* 0x22,*/ 0xBE, 0x05, 0x6C, 0x6C, 0x13, 0x39, 0x5D,
    /* 0x23,*/ 0xA7, 0x4D, 0x00, 0x69, 0x99, 0x26, 0x03,
    /* 0x24,*/ 0x00, 0x00, 0x00, 0x00, 0x13, 0xFC, 0x09,
    /* 0x25,*/ 0x62, 0x00, 0xB5, 0x00, 0x08, 0x14, 0x00,
    /* 0x26,*/ 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };

  GnssPositionData pos(data, sizeof(data));

  EXPECT_TRUE(pos.valid());
  EXPECT_TRUE(pos.latitude().defined());
  EXPECT_TRUE(pos.longitude().defined());
  EXPECT_NEAR(41.37972459, pos.latitude().get().degrees(), 1e-8);
  EXPECT_NEAR(2.1857592038, pos.longitude().get().degrees(), 1e-8);
  EXPECT_NEAR(52.861289, pos.altitude().get().meters(), 1e-6);
}

