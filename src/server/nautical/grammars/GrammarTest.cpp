/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>

#include <server/nautical/grammars/Grammar.h>
#include <server/nautical/grammars/Grammar001.h>

using namespace sail;

TEST(GrammarTest, Grammar001Info) {
  Grammar001 g; //Grammar001Settings());
  Array<HNode> info = g.nodeInfo();
  EXPECT_EQ(info[24].label(), "Off");
  EXPECT_EQ(info.size(), 41);
  EXPECT_EQ(info[29].index(), 29);
}
