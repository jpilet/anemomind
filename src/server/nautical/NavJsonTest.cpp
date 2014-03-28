/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/NavJson.h>
#include <server/common/string.h>

using namespace sail;

TEST(NavJsonTest, ConvertToJson) {
  Nav nav;
  Array<Nav> navs(1, &nav);
  Poco::JSON::Array data = json::encode(navs);
  stringstream ss;
  data.stringify(ss, 0, 0);
  std::string s = ss.str();
  int len = s.length();
  EXPECT_GE(len, 0);
  EXPECT_EQ(s[0], '[');
  EXPECT_EQ(s[len-1], ']');
  const char expected[] = "[{\"alt-m\":nan,\"awa-rad\":nan,\"aws-mps\":nan,\"gps-bearing-rad\":nan,\"gps-speed-mps\":nan,\"lat-rad\":nan,\"lon-rad\":nan,\"maghdg-rad\":nan,\"time-since-1970-s\":nan,\"wat-speed-mps\":nan}]";
  EXPECT_EQ(s, expected);
}
