/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/NavJson.h>
#include <server/common/string.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/ParseHandler.h>

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
  const char expected[] = "[{\"time-int64\":9223372036854775807}]";
  EXPECT_EQ(s, expected);
}




TEST(NavJsonTest, ConvertFromJson) {
  const char dataToDecode[] = "[{\"alt-m\":0.4,\"awa-rad\":0.5235987755982988,\"aws-mps\":6,\"gpsbearing-mps\":2.520777777777778,\"gpsspeed-mps\":2.520777777777778,\"lat-rad\":0.6806784082777885,\"lon-rad\":0.8377580409572782,\"maghdg-rad\":1.5707963267948966,\"time-int64\":1396029819000,\"watspeed-mps\":2.5722222222222224}]";

  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());

  parser.setHandler(handler);

  parser.parse(dataToDecode);
  Poco::Dynamic::Var result = handler->asVar();
  EXPECT_TRUE(result.isArray());
  Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

  sail::Array<Nav> navs;
  json::decode(*arr, &navs);
  EXPECT_EQ(navs.size(), 1);

  std::stringstream ss;
  json::encode(navs).stringify(ss, 0, 0);
  EXPECT_EQ(ss.str(), dataToDecode);
}

TEST(NavJsonTest, ConvertFromJsonUndef) {
  const char dataToDecode[] = "[{\"time-int64\":9223372036854775807}]";


  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());

  parser.setHandler(handler);

  parser.parse(dataToDecode);
  Poco::Dynamic::Var result = handler->asVar();
  EXPECT_TRUE(result.isArray());
  Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

  sail::Array<Nav> navs;
  json::decode(*arr, &navs);
  EXPECT_EQ(navs.size(), 1);

  std::stringstream ss;
  json::encode(navs).stringify(ss, 0, 0);
  EXPECT_EQ(ss.str(), dataToDecode);
}
