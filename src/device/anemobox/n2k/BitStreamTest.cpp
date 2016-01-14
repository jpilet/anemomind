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
