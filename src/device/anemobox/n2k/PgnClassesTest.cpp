/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <device/anemobox/n2k/PgnClasses.h>
#include <device/anemobox/n2k/BitStream.h>
#include <device/anemobox/Nmea2000Utils.h>

using namespace PgnClasses;

template <typename T>
T recode(const T& x) {
  auto data = x.encode();
  return T(data.data(), data.size());
}


TEST(PgnClassesTest, DefaultWindData) {
  PgnClasses::WindData x;
  EXPECT_FALSE(x.hasAllData());
  EXPECT_FALSE(recode(x).hasAllData());
}

TEST(PgnClassesTest, InvalidWindData) {
  uint8_t data[] = {0xFF};

  PgnClasses::WindData x(data, 1);
  EXPECT_FALSE(x.hasAllData());
  EXPECT_FALSE(recode(x).hasAllData());
}

void testWindData(const WindData& windData) {
  EXPECT_TRUE(windData.hasAllData());
  EXPECT_NEAR(windData.windSpeed.get().metersPerSecond(), 0.25, 0.01);
  EXPECT_NEAR(windData.windAngle.get().radians(), 3.0892, 0.0001);
  EXPECT_EQ(windData.reference.get(),  PgnClasses::WindData::Reference::Apparent);
}

TEST(PgnClassesTest, WindData) {
  // The length of WindData is 6 bytes.
  std::vector<uint8_t> data{0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA/*, 0xFF, 0xFF*/};

  PgnClasses::WindData windData(data.data(), data.size());
  testWindData(windData);
  testWindData(recode(windData));
  EXPECT_EQ(data, windData.encode());
}

void testWindDataNotAvailable(const WindData& windData) {
  EXPECT_FALSE(windData.hasAllData());
  EXPECT_FALSE(windData.reference.defined());
}

TEST(PgnClassesTest, WindDataNotAvailable) {
  std::vector<uint8_t> data{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF/*, 0xFF, 0xFF*/};

  PgnClasses::WindData windData(data.data(), data.size());
  testWindDataNotAvailable(windData);
  testWindDataNotAvailable(recode(windData));
  EXPECT_EQ(data, windData.encode());
}

uint8_t shortSrc = 119;

class TestWindVisitor : public PgnClasses::PgnVisitor {
  protected:
    bool apply(const tN2kMsg& c, const PgnClasses::WindData& packet) override {
      if (!packet.hasAllData()) {
        return false;
      }
      if (c.Source != shortSrc) {
        return false;
      }

      EXPECT_TRUE(packet.windSpeed.defined());
      EXPECT_NEAR(packet.windSpeed.get().metersPerSecond(), 0.25, 0.01);
      EXPECT_TRUE(packet.windAngle.defined());
      EXPECT_NEAR(packet.windAngle.get().radians(), 3.0892, 0.0001);
      return true;
    }
};

TEST(PgnClassesTest, WindVisitor) {
  std::vector<uint8_t> data{0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA, 0xFF, 0xFF};
  TestWindVisitor visitor;

  sail::N2kMsgBuilder builder;
  builder.PGN = PgnClasses::WindData::ThisPgn;
  builder.source = shortSrc;
  builder.destination = 83;
  EXPECT_TRUE(visitor.visit(builder.make(data)));
}

void testPositionRapidUpdate(const PositionRapidUpdate& pru) {
  EXPECT_TRUE(pru.latitude.defined());
  EXPECT_TRUE(pru.longitude.defined());
  EXPECT_NEAR(41.3797185, pru.latitude.get().degrees(), 1e-7);
  EXPECT_NEAR(2.1857485, pru.longitude.get().degrees(), 1e-7);
}

TEST(PgnClassesTest, PositionRapidUpdate) {
  std::vector<uint8_t> data{ 0x41, 0x0b, 0xaa, 0x18, 0xcd, 0x84, 0x4d, 0x01 };

  PositionRapidUpdate pru(data.data(), data.size());
  testPositionRapidUpdate(pru);
  testPositionRapidUpdate(recode(pru));
  EXPECT_EQ(data, pru.encode());
}

void testGnssPositionData(const GnssPositionData& pos) {
  EXPECT_TRUE(pos.hasAllData());
  EXPECT_TRUE(pos.latitude.defined());
  EXPECT_TRUE(pos.longitude.defined());
  EXPECT_NEAR(41.37972459, pos.latitude.get().degrees(), 1e-8);
  EXPECT_NEAR(2.1857592038, pos.longitude.get().degrees(), 1e-8);
  EXPECT_NEAR(52.861289, pos.altitude.get().meters(), 1e-6);
}

TEST(PgnClassesTest, GnssPositionData) {
  std::vector<uint8_t> data{
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

  GnssPositionData pos(data.data(), data.size());
  testGnssPositionData(pos);
  testGnssPositionData(recode(pos));
  auto data2 = pos.encode();

  int minLength = std::min(data.size(), data2.size());
  for (int i = 0; i < minLength; i++) {
    if (data[i] != data2[i]) {
      std::cout << "At " << i << ":" << std::endl;
    }
    EXPECT_EQ(data[i], data2[i]);
  }
}

