#include <device/anemobox/n2k/BitStream.h>
#include <gtest/gtest.h>


TEST(BitStreamTest, read4And8) {
  uint8_t data[] = { 0x21, 0x43, 0x65 };
  BitStream stream(data, 3);

  EXPECT_EQ(0x1, stream.getUnsigned(4));

  // This read crosses the boundary of byte 1 and byte 2.
  EXPECT_EQ(0x32, stream.getUnsigned(8));
  EXPECT_EQ(0x4, stream.getUnsigned(4));
  EXPECT_EQ(0x5, stream.getUnsigned(4));
  EXPECT_EQ(0x6, stream.getUnsigned(4));
  EXPECT_FALSE(stream.canRead(1));
}

TEST(BitStreamTest, Unsigned_2_and_3) {
  uint8_t data[] = { 0xC2 };
  BitStream stream(data, 1);

  /*
   * Read unsigned number 2 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits);
    EXPECT_EQ(2, streamValue);
    EXPECT_EQ(63, BitStreamInternals::getMaxUnsignedValue(bits));

    /* Corresponding call to canboat:
    int startBit = 0;
    Field field;
    field.offset = 0;
    field.hasSign = false;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxUnsignedValue(bits));*/
  }/*
    Read unsigned number 3 stored in 2 bits
  */{
    int bits = 2;

    auto streamValue = stream.getUnsigned(bits);
    EXPECT_EQ(3, streamValue);
    EXPECT_EQ(3, BitStreamInternals::getMaxUnsignedValue(bits));

    /* Corresponding call to canboat:
    int startBit = 6;
    Field field;
    field.offset = 0;
    field.hasSign = false;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxUnsignedValue(bits));*/
  }
}

TEST(BitStreamTest, Unsigned_7_and_9) {
  uint8_t data[] = { 0x47, 0xF2};
  BitStream stream(data, 2);
  /*
   * Read unsigned number 7 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits);
    EXPECT_EQ(7, streamValue);
    EXPECT_EQ(63, BitStreamInternals::getMaxUnsignedValue(bits));

    /* Corresponding call to canboat:
    int startBit = 0;
    Field field;
    field.offset = 0;
    field.hasSign = false;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxUnsignedValue(bits));*/
  }/*
    Read unsigned number 9 stored in 6 bits
  */{
    int bits = 6;

    auto streamValue = stream.getUnsigned(bits);
    EXPECT_EQ(9, streamValue);
    EXPECT_EQ(63, BitStreamInternals::getMaxUnsignedValue(bits));

    /* Corresponding call to canboat:
    int startBit = 6;
    Field field;
    field.offset = 0;
    field.hasSign = false;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxUnsignedValue(bits));*/
  }
}

TEST(BitStreamTest, Signed_positive_2_and_1) {
  uint8_t data[] = { 0x42 };
  BitStream stream(data, 1);

  /*
   * Read signed number 2 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(2, streamValue);
    EXPECT_EQ(31, BitStreamInternals::getMaxSignedValueTwosComplement(bits));

    /* Corresponding call to canboat:
    int startBit = 0;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }/*
    Read unsigned number 1 stored in 2 bits
  */{
    int bits = 2;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(1, streamValue);
    EXPECT_EQ(1, BitStreamInternals::getMaxSignedValueTwosComplement(bits));

    /* Corresponding call to canboat:
    int startBit = 6;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }
}

TEST(BitStreamTest, Signed_negative_2_and_1) {
  uint8_t data[] = { 0xFE };
  BitStream stream(data, 1);

  /*
   * Read signed number -2 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(-2, streamValue);
    EXPECT_EQ(31, BitStreamInternals::getMaxSignedValueTwosComplement(bits));

    /* Corresponding call to canboat:
    int startBit = 0;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }/*
    Read unsigned number -1 stored in 2 bits
  */{
    int bits = 2;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(-1, streamValue);
    EXPECT_EQ(1, BitStreamInternals::getMaxSignedValueTwosComplement(bits));

    /* Corresponding call to canboat:
    int startBit = 6;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }
}


TEST(BitStreamTest, Signed_7_and_9) {
  uint8_t data[] = { 0x47, 0xF2};
  BitStream stream(data, 2);
  /*
   * Read signed number 7 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(7, streamValue);
    EXPECT_EQ(31, BitStreamInternals::getMaxSignedValueTwosComplement(bits));

    /* Corresponding call to canboat:
    int startBit = 0;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }/*
    Read signed number 9 stored in 6 bits
  */{
    int bits = 6;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(9, streamValue);
    EXPECT_EQ(31, BitStreamInternals::getMaxSignedValueTwosComplement(bits));

    /* Corresponding call to canboat:
    int startBit = 6;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }
}






TEST(BitStreamTest, Signed_negative_7_and_9) {
  uint8_t data[] = { 0xF9, 0xFD};
  BitStream stream(data, 2);
  /*
   * Read signed number -7 stored in 6 bits
   */
  {
    int bits = 6;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(-7, streamValue);
    EXPECT_EQ(31, BitStreamInternals::getMaxSignedValueTwosComplement(bits));


    /* Corresponding call to canboat:
    int startBit = 0;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }/*
    Read signed number -9 stored in 6 bits
  */{
    int bits = 6;

    auto streamValue = stream.getSigned(bits);
    EXPECT_EQ(-9, streamValue);
    EXPECT_EQ(31, BitStreamInternals::getMaxSignedValueTwosComplement(bits));


    /* Corresponding call to canboat:
    int startBit = 6;
    Field field;
    field.offset = 0;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }
}

TEST(BitStreamTest, Signed_negative_7_with_offset) {
  uint8_t data[] = { 0x01};

  int64_t offset = -8;
  BitStream stream(data, 1);
  /*
   * Read signed number -7 = 1 + offset, with offset = -8
   */
  {
    int bits = 6;

    auto streamValue = stream.getSigned(bits, offset);
    EXPECT_EQ(-7, streamValue);

    /* Corresponding call to canboat:
    int startBit = 0;
    Field field;
    field.offset = offset;
    field.hasSign = true;
    int64_t value = 0;
    int64_t maxValue;
    extractNumber(&field, data, startBit, bits, &value, &maxValue);
    EXPECT_EQ(value, streamValue);
    EXPECT_EQ(maxValue, BitStreamInternals::getMaxSignedValueTwosComplement(bits));*/
  }
}

TEST(BitStreamTest, Max_value_at_the_limit) {
  constexpr int limit = 64;
  uint64_t expectedUnsignedMax = ~static_cast<uint64_t>(0);
  uint64_t expectedSignedMax = expectedUnsignedMax >> 1;
  EXPECT_EQ(expectedUnsignedMax, BitStreamInternals::getMaxUnsignedValue(limit));
  EXPECT_EQ(expectedSignedMax, BitStreamInternals::getMaxSignedValueTwosComplement(limit));
}
