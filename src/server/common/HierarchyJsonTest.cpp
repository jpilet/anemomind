/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */


#include <server/common/HierarchyJson.h>
#include <server/common/HNodeGroup.h>
#include <gtest/gtest.h>
#include <server/common/Json.h>
#include <sstream>

#include <server/common/Json.impl.h>
#include <server/common/JsonIO.h>

using namespace sail;

TEST(HierarchyJsonTest, TestSimple) {
  HNode x(1, 2, "m", "testnode");
  Poco::Dynamic::Var obj = json::serialize(x);
  HNode y;
  json::deserialize(obj, &y);
  EXPECT_EQ(x.index(), y.index());
  EXPECT_EQ(x.parent(), y.parent());
  EXPECT_EQ(x.description(), y.description());
}

TEST(HierarchyJsonTest, TestSimpleArray) {
  HNode x(1, 2, "m", "testnode");
  Array<HNode> X = Array<HNode>{x};
  Poco::Dynamic::Var obj = json::serialize(X);
  Array<HNode> Y;
  json::deserialize(obj, &Y);

  HNode y = Y.first();

  EXPECT_EQ(x.index(), y.index());
  EXPECT_EQ(x.parent(), y.parent());
  EXPECT_EQ(x.description(), y.description());
}




namespace {

Hierarchy makeMiniSailGrammar() {
  return HNodeGroup(8, "Sailing",
      HNodeGroup(6, "PortTack",
          HNodeGroup(0, "Port tack / Close hauled") +
          HNodeGroup(1, "Port tack / Beam reach") +
          HNodeGroup(2, "Port tack / Broad reach")
      )
      +
      HNodeGroup(7, "Starboard tack",
          HNodeGroup(3, "Starboard tack / Broad reach") +
          HNodeGroup(4, "Starboard tack / Beam reach") +
          HNodeGroup(5, "Starboard tack / Close hauled")
      )
  ).compile("mja-%03d");
}


}

TEST(HierarchyJsonTest, HTreeJson) {
  const int len = 18;


  // Graphical explanation of the parsing
  // Depth 0:        [-------------------------8--------------------------]
  // Depth 1:        [----------6--------][--------7-------][------6------]
  // Depth 2:        [-0--][--1-][---2---][-3--][--4-][-5--][------0------]
  int toParse[len] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 0, 0, 0, 0, 0};


  Hierarchy h = makeMiniSailGrammar();
  std::shared_ptr<HTree> X = h.parse(Arrayi(len, toParse));

  Poco::Dynamic::Var obj = json::serialize(X);

  std::shared_ptr<HTree> Y;
  json::deserialize(obj, &Y);

  std::stringstream ssX, ssY;
  X->disp(&ssX);
  Y->disp(&ssY);
  EXPECT_EQ(ssX.str(), ssY.str());
}


