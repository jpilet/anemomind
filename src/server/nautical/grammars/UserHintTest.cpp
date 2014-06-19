/*
 *  Created on: 2014-06-19
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/UserHint.h>
#include <server/nautical/grammars/UserHintJson.h>
#include <gtest/gtest.h>

#include <Poco/JSON/ParseHandler.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>

using namespace sail;

TEST(UserHintTest, UserHintUndefined) {
  UserHint h;
  EXPECT_EQ(h.type(), UserHint::UNDEFINED);
}

TEST(UserHintTest, UserHintRaceStart) {
  TimeStamp t = TimeStamp::UTC(2014, 06, 19, 15, 47, 0);
  UserHint h(UserHint::RACE_START, t);
  EXPECT_EQ(h.type(), UserHint::RACE_START);
  EXPECT_EQ(h.typeAsString(), "race-start");
  EXPECT_EQ(h.time(), t);
}

TEST(UserHintTest, UserHintJson) {
  TimeStamp t = TimeStamp::UTC(2014, 06, 19, 15, 47, 0);
  UserHint h(UserHint::RACE_START, t);

  UserHint h2;
  json::deserialize(json::serialize(h), &h2);
  EXPECT_EQ(h, h2);
}

TEST(UserHintTest, UserHintJson2) {
  TimeStamp t = TimeStamp::UTC(2014, 06, 19, 15, 47, 0);
  UserHint h(UserHint::RACE_START, t);

  std::stringstream ss;
  Poco::JSON::Stringifier::stringify(json::serialize(h), ss);

  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());
  parser.setHandler(handler);
  parser.parse(ss.str());

  UserHint h2;
  EXPECT_TRUE(json::deserialize(handler->asVar(), &h2));
  EXPECT_EQ(h, h2);
}
