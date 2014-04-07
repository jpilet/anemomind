/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>

#include <server/nautical/grammars/Grammar.h>
#include <server/nautical/grammars/Grammar001.h>

using namespace sail;

TEST(GrammarTest, Inst001) {
  Grammar001 g; //Grammar001Settings());
  Array<GrammarNodeInfo> info = g.nodeInfo();
  EXPECT_EQ(info[36].description(), "Off");
}
