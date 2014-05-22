/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */


#include <server/common/HierarchyJson.h>
#include <gtest/gtest.h>
#include <server/common/Json.h>
#include <sstream>

using namespace sail;

TEST(HierarchyJsonTest, TestSimple) {
  HNode x(1, 2, "m", "testnode");
  json::CommonJson::Ptr obj = json::serialize(x);
  HNode y;
  json::deserialize(obj, &y);
  EXPECT_EQ(x.index(), y.index());
  EXPECT_EQ(x.parent(), y.parent());
  EXPECT_EQ(x.description(), y.description());
}

TEST(HierarchyJsonTest, TestSimpleArray) {
  HNode x(1, 2, "m", "testnode");
  Array<HNode> X = Array<HNode>::args(x);
  json::CommonJson::Ptr obj = json::serialize(X);
  Array<HNode> Y;
  json::deserialize(obj, &Y);

  HNode y = Y.first();

  EXPECT_EQ(x.index(), y.index());
  EXPECT_EQ(x.parent(), y.parent());
  EXPECT_EQ(x.description(), y.description());
}




namespace {

Hierarchy makeMiniSailGrammar() {
  Array<HNode> nodes(9);

  // TERMINAL SYMBOLS
  // Port tack
  HNodeFamily fam("mja");
  nodes[0] = fam.make(0, 6, "Port tack / Close hauled");
  nodes[1] = fam.make(1, 6, "Port tack / Beam reach");
  nodes[2] = fam.make(2, 6, "Port tack / Broad reach");
  // Starboard tack
  nodes[3] = fam.make(3, 7, "Starboard tack / Broad reach");
  nodes[4] = fam.make(4, 7, "Starboard tack / Beam reach");
  nodes[5] = fam.make(5, 7, "Starboard tack / Close hauled");

  // GROUPING SYMBOLS
  nodes[6] = fam.make(6, 8, "Port tack");
  nodes[7] = fam.make(7, 8, "Starboard tack");
  nodes[8] = fam.makeRoot(8, "Sailing");
  return Hierarchy(nodes);
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

  json::CommonJson::Ptr obj = json::serialize(X);

  std::shared_ptr<HTree> Y;
  json::deserialize(obj, &Y);

  std::stringstream ssX, ssY;
  X->disp(&ssX);
  Y->disp(&ssY);
  EXPECT_EQ(ssX.str(), ssY.str());
}


