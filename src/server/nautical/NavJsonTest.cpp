/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/NavJson.h>
#include <server/common/string.h>

using namespace sail;

TEST(NavJsonTest, ConvertToJson) {
  Nav nav;
  Array<Nav> navs(1, &nav);
  Poco::JSON::Object::Ptr data = convertToJson(navs);
  stringstream ss;
  data->stringify(ss, 0, 0);
  std::string s = ss.str();
  int len = s.length();
  EXPECT_GE(len, 0);
  EXPECT_EQ(s[0], '{');
  EXPECT_EQ(s[len-1], '}');

  const char expected[] = "{\"data\":[[1,nan,nan,nan,nan,nan,nan,nan,nan,nan,nan]],\"format\":[\"format-version\",\"time-since-1970-seconds\",\"pos-longitude-radians\",\"pos-latitude-radians\",\"pos-altitude-meters\",\"awa-radians\",\"aws-meters-per-second\",\"maghdg-radians\",\"wat-speed-meters-per-second\",\"gps-speed-meters-per-second\",\"gps-bearing-radians\"]}";

  EXPECT_EQ(s, expected);
}
