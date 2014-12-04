/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>

#include <server/nautical/grammars/Grammar.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <iostream>

using namespace sail;

TEST(GrammarTest, Grammar001Info) {


  WindOrientedGrammarSettings settings;

  WindOrientedGrammar g(settings);
  Array<std::string> descriptions = HNode::makeDescriptionList(g.nodeInfo());
  for (int i = 0; i < descriptions.size(); i++) {
    std::cout << /*i+1 << ". " <<*/ descriptions[i] << std::endl;
  }

  Array<HNode> info = g.nodeInfo();
  EXPECT_EQ(info[24].description(), "Off");
  EXPECT_EQ(info.size(), 41);
  EXPECT_EQ(info[29].index(), 29);
}
