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
  const char expected[] = "[{\"time-milliseconds-since-1970\":9223372036854775807}]";
  EXPECT_EQ(s, expected);
}





void runJsonEncDecTest(const char *dataToDecode) {


  sail::Array<Nav> navs1;
  {
    Poco::JSON::Parser parser;
    Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());

    parser.setHandler(handler);
    parser.parse(dataToDecode);
    Poco::Dynamic::Var result = handler->asVar();
    EXPECT_TRUE(result.isArray());
    Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

    json::decode(*arr, &navs1);
    EXPECT_EQ(navs1.size(), 1);
  }

  std::stringstream ss;
  json::encode(navs1).stringify(ss, 0, 0);
  std::string dataToDecode2 = ss.str();

  {
     Poco::JSON::Parser parser;
     Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());

     parser.setHandler(handler);
     parser.parse(dataToDecode2);
     Poco::Dynamic::Var result = handler->asVar();
     EXPECT_TRUE(result.isArray());
     Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

     Array<Nav> navs2;
     json::decode(*arr, &navs2);
     EXPECT_EQ(navs2.size(), 1);
     EXPECT_EQ(navs1[0], navs2[0]);
  }
}

TEST(NavJsonTest, EncDecTest) {
  {
    const char dataToDecode[] = "[{\"time-milliseconds-since-1970\":9223372036854775807}]";
    runJsonEncDecTest(dataToDecode);
  }
  {
    const char dataToDecode[] = "[{\"alt-m\":0.4,\"awa-rad\":0.5235987755982988,\"aws-mps\":6,\"gpsbearing-mps\":2.520777777777778,\"gpsspeed-mps\":2.520777777777778,\"lat-rad\":0.6806784082777885,\"lon-rad\":0.8377580409572782,\"maghdg-rad\":1.5707963267948966,\"time-milliseconds-since-1970\":1396029819000,\"watspeed-mps\":2.5722222222222224}]";
    runJsonEncDecTest(dataToDecode);
  }
}
