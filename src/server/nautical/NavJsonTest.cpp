/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <Poco/JSON/Array.h>
#include <Poco/JSON/ParseHandler.h>
#include <Poco/JSON/Parser.h>
#include <gtest/gtest.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/nautical/NavJson.h>

using namespace sail;

TEST(NavJsonTest, ConvertToJson) {
  Nav nav;
  Array<Nav> navs(1, &nav);
  json::CommonJson::Ptr data = json::serialize(navs);
  stringstream ss;
  data->stringify(ss, 0, 0);
  std::string s = ss.str();
  int len = s.length();
  EXPECT_GE(len, 0);
  EXPECT_EQ(s[0], '[');
  EXPECT_EQ(s[len-1], ']');
  const char expected[] = "[{\"time_ms_1970\":9223372036854775807}]";
  EXPECT_EQ(s, expected);
}


namespace {

Array<Nav> deserializeNavs(const char *dataToDecode) {
  sail::Array<Nav> navs;
  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());

  parser.setHandler(handler);
  try {
    parser.parse(dataToDecode);
  } catch (Poco::Exception &e) {
    LOG(FATAL) << e.displayText() << "\nFor JSON: " << dataToDecode;
  }
  Poco::Dynamic::Var result = handler->asVar();
  EXPECT_TRUE(result.isArray());
  Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

  json::deserialize(*arr, &navs);
  return navs;
}

void runJsonEncDecTest(const char *dataToDecode) {
  sail::Array<Nav> navs = deserializeNavs(dataToDecode);
  EXPECT_EQ(navs.size(), 1);

  std::stringstream ss;
  json::serialize(navs).stringify(ss, 0, 0);

  Array<Nav> navs2 = deserializeNavs(ss.str().c_str());
  EXPECT_EQ(navs2.size(), 1);
  EXPECT_EQ(navs[0], navs2[0]);
}

}  // namespace

TEST(NavJsonTest, EncDecTest) {
  runJsonEncDecTest(
      "[{\"time_ms_1970\":9223372036854775807}]");
  runJsonEncDecTest(
    "[{\"alt_m\":0.4,"
    "\"awa_rad\":0.5235987755982988,"
    "\"aws_mps\":6,"
    "\"gpsbearing_rad\":-0.3,"
    "\"gpsspeed_mps\":1.1,"
    "\"lat_rad\":0.6806784082777885,"
    "\"lon_rad\":0.8377580409572782,"
    "\"maghdg_rad\":-0.301,"
    "\"time_ms_1970\":1396029819000,"
    "\"watspeed_mps\":0.03}]");
}

TEST(NavJsonTest, BackwardCompatibilityTest) {
  // Make sure the following format can be correctly de-serialized.
  const char dataToDecode[] =
    "[{\"alt_m\":0.4,"
    "\"awa_rad\":0.5235987755982988,"
    "\"aws_mps\":6,"
    "\"gpsbearing_rad\":-0.3,"
    "\"gpsspeed_mps\":1.1,"
    "\"lat_rad\":0.6806784082777885,"
    "\"lon_rad\":0.8377580409572782,"
    "\"maghdg_rad\":-0.301,"
    "\"time_ms_1970\":1396029819000,"
    "\"watspeed_mps\":0.03}]";
  Nav base;
  base.setGeographicPosition(
      GeographicPosition<double>(
          Angle<>::radians(0.8377580409572782),
          Angle<>::radians(0.6806784082777885),
          Length<>::meters(0.4)));
  base.setAwa(Angle<>::radians(0.5235987755982988));
  base.setAws(Velocity<>::metersPerSecond(6));
  base.setGpsBearing(Angle<>::radians(-.3));
  base.setGpsSpeed(Velocity<>::metersPerSecond(1.1));
  base.setMagHdg(Angle<>::radians(-.301));
  base.setTime(TimeStamp::fromMilliSecondsSince1970(1396029819000));
  base.setWatSpeed(Velocity<>::metersPerSecond(.03));

  // Both objects should be the same.
  Array<Nav> deserialized = deserializeNavs(dataToDecode);
  EXPECT_EQ(deserialized[0], base);
}
