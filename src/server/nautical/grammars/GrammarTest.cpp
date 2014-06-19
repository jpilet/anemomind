/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>

#include <server/nautical/grammars/Grammar.h>
#include <server/nautical/grammars/Grammar001.h>

using namespace sail;

TEST(GrammarTest, Grammar001Info) {


  Grammar001Settings settings;

  Grammar001 g(settings);

  Array<HNode> info = g.nodeInfo();
  EXPECT_EQ(info[24].description(), "Off");
  EXPECT_EQ(info.size(), 41);
  EXPECT_EQ(info[29].index(), 29);
}

/*
 * Checks that the new way of defining the grammar
 * using HNodeGroup does not break anything.
 */
TEST(GrammarTest, Grammar001HierarchyTransitionTest) {
  EXPECT_TRUE(grammar001HierarchEquivalence());
}
