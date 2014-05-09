/*
 *  Created on: 2014-04-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/string.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(StringTest, HexDigit) {
  const char digits[] = "0123456789ABCDEF";
  for (int i = 0; i < 16; i++) {
    EXPECT_EQ(i, decodeHexDigit(digits[i]));
  }
}

TEST(StringTest, HexDigitLC) {
  const char digits[] = "0123456789abcdef";
  for (int i = 0; i < 16; i++) {
    EXPECT_EQ(i, decodeHexDigit(digits[i]));
  }
}

TEST(StringTest, ToLower) {
  std::string src = "ANEMOMIND";
  std::string dst = "anemomind";
  EXPECT_EQ(toLower(src), dst);
}


TEST(StringTest, FormatTest) {
  std::string result = stringFormat("Nine is %d", 9);
  EXPECT_EQ(result, "Nine is 9");
}

TEST(StringTest, Int64Test) {
  int64_t x = 0x1234567890ABCDEF;
  EXPECT_EQ("1234567890ABCDEF", int64ToHex(x));
}

TEST(StringTest, Int64TestOrdering) {
  int64_t a = 234;
  int64_t b = 3456;

  EXPECT_LE(a, b);
  EXPECT_LE(int64ToHex(a), int64ToHex(b));
}
