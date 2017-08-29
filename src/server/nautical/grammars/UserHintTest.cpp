/*
 *  Created on: 2014-06-19
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/UserHint.h>
#include <gtest/gtest.h>

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
