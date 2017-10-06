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

TEST(StringTetst, joinTest) {
  EXPECT_EQ("a+b+c", join(std::vector<std::string>{"a", "b", "c"}, "+"));
  EXPECT_EQ("a", join(std::vector<std::string>{"a"}, "\n"));
}
