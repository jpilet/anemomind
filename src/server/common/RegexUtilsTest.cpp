/*
 * RegexUtilsTest.cpp
 *
 *  Created on: 6 Apr 2018
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/RegexUtils.h>

using namespace sail;

bool matches(const std::string& s, const std::string& pattern, bool dbg = false) {
  std::smatch m;
  std::regex re(pattern);
  if (dbg) {
    std::cout << "INPUT:   " << s << std::endl;
    std::cout << "PATTERN: " << pattern << std::endl;
  }
  return std::regex_match(s, m, re);
}

TEST(RegexUtilsTest, TestIt) {
  using namespace Regex;
  EXPECT_TRUE(matches("", entireString("")));
  EXPECT_FALSE(matches("  ", entireString("")));
  EXPECT_TRUE(matches("  ", entireString(anyCount(space))));
  EXPECT_FALSE(matches(
      "   9   ",
      entireString(anyCount(space))));
  EXPECT_TRUE(matches(
      "   9   ",
      entireString(anyCount(space)/unsignedInteger()/anyCount(space))));
  EXPECT_TRUE(matches(
      "   933   ",
      entireString(anyCount(space)/unsignedInteger()/anyCount(space))));
  EXPECT_FALSE(matches(
      "   933.3   ",
      entireString(anyCount(space)/unsignedInteger()/anyCount(space))));
  EXPECT_TRUE(matches(
      "933.3",
      entireString(unsignedFractionalNumber(digit))));
  EXPECT_FALSE(matches(
      "93",
      entireString(unsignedFractionalNumber(digit))));
  EXPECT_TRUE(matches(
      "-93",
      entireString(basicNumber(digit))));
  EXPECT_TRUE(matches(
      "+93",
      entireString(basicNumber(digit))));
  EXPECT_TRUE(matches(
      "+.93",
      entireString(basicNumber(digit))));
  EXPECT_TRUE(matches(
      "-.93",
      entireString(basicNumber(digit))));
  EXPECT_TRUE(matches(
      "+34.93",
      entireString(basicNumber(digit))));
  EXPECT_TRUE(matches(
      "-34.93",
      entireString(basicNumber(digit))));
  EXPECT_TRUE(matches(
      "+34.",
      entireString(basicNumber(digit))));
}



