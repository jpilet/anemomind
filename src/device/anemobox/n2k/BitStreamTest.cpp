#include <device/anemobox/n2k/BitStream.h>
#include <gtest/gtest.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

const int N = 3;
uint8_t data[N] = { 0x21, 0x43, 0x65 };

TEST(BitStreamTest, read4And8) {
  BitStream stream(data, N);

  EXPECT_EQ(0x1, stream.getUnsigned(4));

  // This read crosses the boundary of byte 1 and byte 2.
  EXPECT_EQ(0x32, stream.getUnsigned(8));
  EXPECT_EQ(0x4, stream.getUnsigned(4));
  EXPECT_EQ(0x5, stream.getUnsigned(4));
  EXPECT_EQ(0x6, stream.getUnsigned(4));
  EXPECT_FALSE(stream.canRead(1));
}

TEST(BitStreamTest, Write4And8) {
  BitOutputStream stream;
  stream.pushUnsigned(4, 0x1);
  stream.pushUnsigned(8, 0x32);
  stream.pushUnsigned(4, 0x4);
  stream.pushUnsigned(4, 0x5);
  stream.pushUnsigned(4, 0x6);

  const auto& d = stream.data();
  EXPECT_EQ(d.size(), N);
  for (int i = 0; i < N; i++) {
    EXPECT_EQ(d[i], data[i]);
  }
}

uint64_t evaluate(const std::vector<bool>& X) {
  uint64_t dst = 0;
  for (auto x: X) {
    dst = 2*dst + (x? 1 : 0);
  }
  return dst;
}

TEST(BitStreamTest, BackAndForth) {
  std::vector<std::vector<bool>> testData{
    {1, 0},
    {0, 1, 1, 1, 1, 0},
    {1, 0, 0, 1, 1, 1, 0, 0, 0},
    {0, 1, 1},
    {1, 0, 0},
    {1, 0, 0, 1, 0},
    {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0},
    {1, 0, 0, 1, 0},
    {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0},
    {1, 0, 0, 1, 0},
    {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0},
    {1, 0, 0, 1, 0},
    {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
    {0, 1, 1},
    {1, 0, 0},
    {0, 1, 1},
    {1, 0, 0},
    {0, 1, 1},
    {1, 0, 0},
    {0, 1, 1},
    {1, 0, 0},
    {0, 1, 1},
    {1, 0, 0}
  };

  int total = 0;
  BitOutputStream dst;
  for (auto x: testData) {
    dst.pushUnsigned(x.size(), evaluate(x));
    total += x.size();
  }
  EXPECT_EQ(dst.lengthBits(), total);

  const auto& d = dst.data();
  BitStream src(d.data(), d.size());
  for (auto x: testData) {
    EXPECT_EQ(src.getUnsigned(x.size()), evaluate(x));
  }
}
