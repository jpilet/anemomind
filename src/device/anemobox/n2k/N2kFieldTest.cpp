/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <device/anemobox/n2k/N2kField.h>

using namespace N2kField;

TEST(N2kFieldTest, Unsigned_2_and_3) {
  uint8_t data[] = { 0xC2 };
  N2kFieldStream stream(data, 1);

  /*
   * Read unsigned number 2 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(2, streamValue());
    EXPECT_EQ(63, getMaxUnsignedValue(bits));
  }/*
    Read unsigned number 3 stored in 2 bits
  */{
    int bits = 2;

    auto streamValue = stream.getUnsigned(bits, Definedness::AlwaysDefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(3, streamValue());
    EXPECT_EQ(3, getMaxUnsignedValue(bits));
  }
}

TEST(N2kFieldTest, Unsigned_7_and_9) {
  uint8_t data[] = { 0x47, 0xF2};
  N2kFieldStream stream(data, 2);
  /*
   * Read unsigned number 7 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(7, streamValue());
    EXPECT_EQ(63, getMaxUnsignedValue(bits));
  }/*
    Read unsigned number 9 stored in 6 bits
  */{
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(9, streamValue());
    EXPECT_EQ(63, getMaxUnsignedValue(bits));
  }
}

TEST(N2kFieldTest, Signed_positive_2_and_1) {
  uint8_t data[] = { 0x42 };
  N2kFieldStream stream(data, 1);

  /*
   * Read signed number 2 stored in 6 bits
   */
  {
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(2, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }/*
    Read unsigned number 1 stored in 2 bits
  */{
    int bits = 2;
    auto offset = 0;
    auto streamValue = stream.getSigned(bits, offset, Definedness::AlwaysDefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(1, streamValue());
    EXPECT_EQ(1, getMaxSignedValue(bits, offset));
  }
}

TEST(N2kFieldTest, Signed_negative_2_and_1) {
  uint8_t data[] = { 0xFE };
  N2kFieldStream stream(data, 1);

  /*
   * Read signed number -2 stored in 6 bits
   */
  {
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(-2, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }/*
    Read unsigned number -1 stored in 2 bits
  */{
    int bits = 2;
    int offset = 0;
    auto streamValue = stream.getSigned(bits, offset, Definedness::AlwaysDefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(-1, streamValue());
    EXPECT_EQ(1, getMaxSignedValue(bits, offset));
  }
}


TEST(N2kFieldTest, Signed_7_and_9) {
  uint8_t data[] = { 0x47, 0xF2};
  N2kFieldStream stream(data, 2);
  /*
   * Read signed number 7 stored in 6 bits
   */
  {
    int offset = 0;
    int bits = 6;

    auto streamValue = stream.getSigned(bits, offset, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(7, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }/*
    Read signed number 9 stored in 6 bits
  */{
    int bits = 6;
    int offset = 0;
    auto streamValue = stream.getSigned(bits, offset, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(9, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }
}






TEST(N2kFieldTest, Signed_negative_7_and_9) {
  uint8_t data[] = { 0xF9, 0xFD};
  N2kFieldStream stream(data, 2);
  /*
   * Read signed number -7 stored in 6 bits
   */
  {
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(-7, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }/*
    Read signed number -9 stored in 6 bits
  */{
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::UndefinedIfMax);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(-9, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }
}

TEST(N2kFieldTest, Signed_negative_7_with_offset) {
  uint8_t data[] = { 0x01};

  int64_t offset = -8;
  N2kFieldStream stream(data, 1);
  /*
   * Read signed number -7 = 1 + offset, with offset = -8
   */
  {
    int bits = 6;

    auto streamValue = stream.getSigned(bits, offset, Definedness::AlwaysDefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(63 + offset, getMaxSignedValue(6, offset));
    EXPECT_EQ(-7, streamValue());
  }
}

TEST(N2kFieldTest, Max_value_at_the_limit) {
  constexpr int limit = 64;
  uint64_t expectedUnsignedMax = ~static_cast<uint64_t>(0);
  uint64_t expectedSignedMax = expectedUnsignedMax >> 1;
  EXPECT_EQ(expectedUnsignedMax, getMaxUnsignedValue(limit));
  EXPECT_EQ(expectedSignedMax, getMaxSignedValue(limit, 0));
  EXPECT_EQ(expectedUnsignedMax - 3, getMaxSignedValue(limit, -3));
}

// Based on the Windows NK2 application connected to
// the Alinghi device.
TEST(N2kFieldTest, ParseDemo) {
  uint8_t data[] = {0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA, 0xFF, 0xFF};
  N2kFieldStream stream(data, 8);

  // PGN: 130306, Wind data

  // Field 1
  auto sid = stream.getUnsigned(8, Definedness::UndefinedIfMax);

  // Field 2
  auto windSpeed =
      stream.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);

  // Field 3
  auto windAngle =
      stream.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);

  /*<EnumPair Value='0' Name='True (ground referenced to North)' />
  <EnumPair Value='1' Name='Magnetic (ground referenced to Magnetic North)' />
  <EnumPair Value='2' Name='Apparent' />
  <EnumPair Value='3' Name='True (boat referenced)' />
  <EnumPair Value='4' Name='True (water referenced)' />*/
  auto reference = stream.getUnsignedInSet(3, {0, 1, 2, 3, 4});

  EXPECT_NEAR(windSpeed().metersPerSecond(), 0.25, 0.01);
  EXPECT_NEAR(windAngle().radians(), 3.0892, 0.0001);
  EXPECT_EQ(reference(), 2);


}

TEST(N2kFieldTest, IntSet0) {
  EXPECT_TRUE(contains({1, 2, 3}, 1));
  EXPECT_TRUE(contains({1, 2, 3}, 2));
  EXPECT_TRUE(contains({1, 2, 3}, 3));
  EXPECT_FALSE(contains({1, 2, 3}, 0));
  EXPECT_FALSE(contains({1, 2, 3}, 4));
}
