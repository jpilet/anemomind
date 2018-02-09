/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <device/anemobox/n2k/N2kField.h>
#include <iostream>

using namespace N2kField;

void testUnsigned_2_and_3(const uint8_t* data, int size) {
  N2kFieldStream stream(data, size);

  /*
   * Read unsigned number 2 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits, Definedness::MaybeUndefined);
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

TEST(N2kFieldTest, Unsigned_2_and_3) {
  uint8_t data[] = { 0xC2 };
  testUnsigned_2_and_3(data, sizeof(data));
}

void testUnsigned_7_and_9(const uint8_t* data, int size) {
  N2kFieldStream stream(data, size);
  /*
   * Read unsigned number 7 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits, Definedness::MaybeUndefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(7, streamValue());
    EXPECT_EQ(63, getMaxUnsignedValue(bits));
  }/*
    Read unsigned number 9 stored in 6 bits
  */{
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits, Definedness::MaybeUndefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(9, streamValue());
    EXPECT_EQ(63, getMaxUnsignedValue(bits));
  }
}


TEST(N2kFieldTest, Unsigned_7_and_9) {
  uint8_t data[] = { 0x47, 0xF2};
  testUnsigned_7_and_9(data, sizeof(data));
}

void testSigned_positive_2_and_1(const uint8_t* data, int size) {
  N2kFieldStream stream(data, size);

  /*
   * Read signed number 2 stored in 6 bits
   */
  {
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::MaybeUndefined);
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

TEST(N2kFieldTest, Signed_positive_2_and_1) {
  uint8_t data[] = { 0x42 };
  testSigned_positive_2_and_1(data, sizeof(data));
}

void testSigned_negative_2_and_1(const uint8_t* data, int size) {
  N2kFieldStream stream(data, size);

  /*
   * Read signed number -2 stored in 6 bits
   */
  {
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::MaybeUndefined);
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

TEST(N2kFieldTest, Signed_negative_2_and_1) {
  uint8_t data[] = { 0xFE };
  testSigned_negative_2_and_1(data, sizeof(data));
}

void testSigned_7_and_9(const uint8_t* data, int size) {
  N2kFieldStream stream(data, size);
  /*
   * Read signed number 7 stored in 6 bits
   */
  {
    int offset = 0;
    int bits = 6;

    auto streamValue = stream.getSigned(bits, offset, Definedness::MaybeUndefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(7, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }/*
    Read signed number 9 stored in 6 bits
  */{
    int bits = 6;
    int offset = 0;
    auto streamValue = stream.getSigned(bits, offset, Definedness::MaybeUndefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(9, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }
}

TEST(N2kFieldTest, Signed_7_and_9) {
  uint8_t data[] = { 0x47, 0xF2};
  testSigned_7_and_9(data, sizeof(data));
}




void testSigned_negative_7_and_9(const uint8_t* data, int size) {
  N2kFieldStream stream(data, size);
  /*
   * Read signed number -7 stored in 6 bits
   */
  {
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::MaybeUndefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(-7, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }/*
    Read signed number -9 stored in 6 bits
  */{
    int bits = 6;
    int offset = 0;

    auto streamValue = stream.getSigned(bits, offset, Definedness::MaybeUndefined);
    EXPECT_TRUE(streamValue.defined());
    EXPECT_EQ(-9, streamValue());
    EXPECT_EQ(31, getMaxSignedValue(bits, offset));
  }
}

TEST(N2kFieldTest, Signed_negative_7_and_9) {
  uint8_t data[] = { 0xF9, 0xFD};
  testSigned_negative_7_and_9(data, sizeof(data));
}

void testSigned_negative_7_with_offset(const uint8_t* data, int size) {
  int64_t offset = -8;
  N2kFieldStream stream(data, size);
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

TEST(N2kFieldTest, Signed_negative_7_with_offset) {
  uint8_t data[] = { 0x01};
  testSigned_negative_7_with_offset(data, sizeof(data));
}

TEST(N2kFieldTest, Max_value_at_the_limit) {
  constexpr int limit = 64;
  uint64_t expectedUnsignedMax = ~static_cast<uint64_t>(0);
  uint64_t expectedSignedMax = expectedUnsignedMax >> 1;
  EXPECT_EQ(expectedUnsignedMax, getMaxUnsignedValue(limit));
  EXPECT_EQ(expectedSignedMax, getMaxSignedValue(limit, 0));
  EXPECT_EQ(expectedUnsignedMax - 3, getMaxSignedValue(limit, -3));
}

void testParseDemo(const uint8_t* data, int size) {
  N2kFieldStream stream(data, size);

  // PGN: 130306, Wind data

  // Field 1
  auto sid = stream.getUnsigned(8, Definedness::MaybeUndefined);

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

// Based on the Windows NK2 application connected to
// the Alinghi device.
TEST(N2kFieldTest, ParseDemo) {
  uint8_t data[] = {0xFF, 0x19, 0x00, 0xAC, 0x78, 0xFA, 0xFF, 0xFF};
  testParseDemo(data, sizeof(data));
}

TEST(N2kFieldTest, IntSet0) {
  EXPECT_TRUE(contains({1, 2, 3}, 1));
  EXPECT_TRUE(contains({1, 2, 3}, 2));
  EXPECT_TRUE(contains({1, 2, 3}, 3));
  EXPECT_FALSE(contains({1, 2, 3}, 0));
  EXPECT_FALSE(contains({1, 2, 3}, 4));
}

TEST(N2kFieldTest, InvalidValues) {
  uint8_t data[] = {
    0xFD, // Reserved
    0xFE, // Out of bounds
    0xFF, // Not available
    0x03  // Valid value
  };
  N2kFieldStream stream(data, sizeof(data));

  EXPECT_FALSE(stream.getUnsigned(8, Definedness::MaybeUndefined).defined());
  EXPECT_FALSE(stream.getUnsigned(8, Definedness::MaybeUndefined).defined());
  EXPECT_FALSE(stream.getUnsigned(8, Definedness::MaybeUndefined).defined());
  EXPECT_TRUE(stream.getUnsigned(8, Definedness::MaybeUndefined).defined());
}



/*****************************
 * Unit tests related to N2kFieldOutputStream
 */

TEST(N2kFieldTest, Write_Unsigned_2_and_3) {
  N2kFieldOutputStream output;
  output.pushUnsigned(6, 2);
  output.pushUnsigned(2, 3);
  auto d = output.moveData();
  testUnsigned_2_and_3(d.data(), d.size());
}

TEST(N2kFieldTest, Write_Unsigned_7_and_9) {
  N2kFieldOutputStream output;
  output.pushUnsigned(6, 7);
  output.pushUnsigned(6, 9);
  auto d = output.moveData();
  testUnsigned_7_and_9(d.data(), d.size());
}

TEST(N2kFieldTest, Write_Signed_positive_2_and_1) {
  N2kFieldOutputStream output;
  output.pushSigned(6, 0, 2);
  output.pushSigned(2, 0, 1);
  auto d = output.moveData();
  testSigned_positive_2_and_1(d.data(), d.size());
}

TEST(N2kFieldTest, Write_Signed_negative_2_and_1) {
  N2kFieldOutputStream output;
  output.pushSigned(6, 0, -2);
  output.pushSigned(2, 0, -1);
  auto d = output.moveData();
  testSigned_negative_2_and_1(d.data(), d.size());
}

TEST(N2kFieldTest, Write_Signed_7_and_9) {
  N2kFieldOutputStream output;
  output.pushSigned(6, 0, 7);
  output.pushSigned(6, 0, 9);
  auto d = output.moveData();
  testSigned_7_and_9(d.data(), d.size());
}

TEST(N2kFieldTest, Write_Signed_negative_7_and_9) {
  N2kFieldOutputStream output;
  output.pushSigned(6, 0, -7);
  output.pushSigned(6, 0, -9);
  auto d = output.moveData();
  testSigned_negative_7_and_9(d.data(), d.size());
}

TEST(N2kFieldTest, Write_Signed_negative_7_with_offset) {
  N2kFieldOutputStream output;
  output.pushSigned(6, -8, -7);
  auto d = output.moveData();
  testSigned_negative_7_with_offset(d.data(), d.size());
}

TEST(N2kFieldTest, Write_parseDemo) {
  N2kFieldOutputStream output;

  using namespace sail;

  output.pushUnsigned(8, 0); // What should sid be? 0 I think.
  output.pushPhysicalQuantity<Velocity<double>>(false, 0.01,
      Velocity<double>::metersPerSecond(1.0),
      16, 0, 0.25_mps);

  output.pushPhysicalQuantity<Angle<double>>(
      false, 0.0001, sail::Angle<double>::radians(1.0),
      16, 0, 3.0892_rad);

  output.pushUnsigned(3, 2);

  auto d = output.moveData();
  testParseDemo(d.data(), d.size());
}

TEST(N2kFieldTest, ReadAndWriteWithResolution) {
  N2kFieldOutputStream output;
  double value = 119.23;
  double resolution = 0.01;
  bool isSigned = true;
  int bits = 15;
  int64_t offset = 0;
  output.pushDoubleWithResolution(
      resolution, isSigned, bits, offset, value);
  auto data = output.moveData();
  N2kFieldStream input(data.data(), data.size());

  auto x = input.getDoubleWithResolution(resolution, isSigned, bits, offset,
      Definedness::AlwaysDefined);
  EXPECT_TRUE(x.defined());
  EXPECT_NEAR(x.get(), value, resolution*1.2);

}
