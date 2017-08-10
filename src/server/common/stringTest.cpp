/*
 *  Created on: 2014-04-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/string.h>
#include <gtest/gtest.h>
#include <server/common/TimeStamp.h>

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

TEST(StringTest, IntegerAsString) {
  IntegerAsString<int> a(0);
  EXPECT_EQ(a.str(), std::string("0"));
  IntegerAsString<int> b(119);
  EXPECT_EQ(b.str(), std::string("119"));
}

TEST(StringTest, BenchmarkIntToString) {
  int n = 1000000;
  uint64_t a = 0;
  uint64_t b = 0;
  {
    TimeStamp start = TimeStamp::now();
    for (int i = 0; i < n; i++) {
      a += IntegerAsString<int>(i).str() - (const char *)nullptr;
    }
    auto e = TimeStamp::now() - start;
    std::cout << "Elapsed optimized: " << e.seconds() << std::endl;
  }{
    TimeStamp start = TimeStamp::now();
    for (int i = 0; i < n; i++) {
      b += objectToString<int>(i).c_str() - (const char *)nullptr;
    }
    auto e = TimeStamp::now() - start;
    std::cout << "Elapsed optimized: " << e.seconds() << std::endl;
  }
  // Just some dummy comparison to make sure that
  // the above loops are not simplified in some way.
  EXPECT_NE(a, b);
}

TEST(StringTetst, joinTest) {
  EXPECT_EQ("a+b+c", join(std::vector<std::string>{"a", "b", "c"}, "+"));
  EXPECT_EQ("a", join(std::vector<std::string>{"a"}, "\n"));
}
