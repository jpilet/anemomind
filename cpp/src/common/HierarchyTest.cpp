/*
 *  Created on: 2014-01-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Hierarchy.h"
#include "gtest/gtest.h"
#include "common.h"

namespace sail {

namespace {
  Array<HNode> makeBinaryTestNodes() {
    Array<HNode> nodes(3);
    nodes[0] = HNode(0, 2, "Zero");
    nodes[1] = HNode(1, 2, "One");
    nodes[2] = HNode::makeRoot(2, "BinaryDigits");
    return nodes;
  }
}

// Check that the root of the hierarchy is correct
TEST(HierarchyTest, RootTest) {
  Array<HNode> nodes = makeBinaryTestNodes();
  Hierarchy hierarchy(nodes);
  EXPECT_EQ(hierarchy.rootNode(), 2);
}

// Check that the level of every node is correct
TEST(HierarchyTest, LevelTest) {
  Array<HNode> nodes = makeBinaryTestNodes();
  Hierarchy hierarchy(nodes);
  EXPECT_EQ(hierarchy.level(0), 1);
  EXPECT_EQ(hierarchy.level(1), 1);
  EXPECT_EQ(hierarchy.level(2), 0);
}

// Check which nodes are terminals
TEST(HierarchyTest, TerminalTest) {
  Array<HNode> nodes = makeBinaryTestNodes();
  Hierarchy hierarchy(nodes);
  EXPECT_TRUE(hierarchy.isTerminal(0));
  EXPECT_TRUE(hierarchy.isTerminal(1));
  EXPECT_FALSE(hierarchy.isTerminal(2));
}

TEST(HierarchyTest, ParseTestBinary) {
  const int len = 8;
  int stringToParse[len] = {0, 0, 0, 0, 1, 1, 1, 0};
  Hierarchy h(makeBinaryTestNodes());
  std::shared_ptr<HTree> tree = h.parse(Arrayi(len, stringToParse));
  EXPECT_EQ(tree->left(), 0);
  EXPECT_EQ(tree->right(), len);
  EXPECT_EQ(tree->childCount(), 3);

  //tree->disp(&std::cout);
  std::shared_ptr<HTree> p0(new HLeaves(0, 0, 4));
  std::shared_ptr<HTree> p1(new HLeaves(4, 1, 3));
  std::shared_ptr<HTree> p2(new HLeaves(7, 0, 1));
  std::shared_ptr<HTree> p4(new HInner(2, p0));
  p4->add(p1);
  p4->add(p2);
  EXPECT_TRUE(tree->child(0)->equals(p0));
  EXPECT_TRUE(tree->child(1)->equals(p1));
  EXPECT_TRUE(tree->child(2)->equals(p2));
  EXPECT_TRUE(p4->equals(tree));
}

Hierarchy makeMiniSailGrammar() {
  Array<HNode> nodes(9);

  // TERMINAL SYMBOLS
    // Port tack
      nodes[0] = HNode(0, 6, "Port tack / Close hauled");
      nodes[1] = HNode(1, 6, "Port tack / Beam reach");
      nodes[2] = HNode(2, 6, "Port tack / Broad reach");
    // Starboard tack
      nodes[3] = HNode(3, 7, "Starboard tack / Broad reach");
      nodes[4] = HNode(4, 7, "Starboard tack / Beam reach");
      nodes[5] = HNode(5, 7, "Starboard tack / Close hauled");

  // GROUPING SYMBOLS
      nodes[6] = HNode(6, 8, "Port tack");
      nodes[7] = HNode(7, 8, "Starboard tack");
      nodes[8] = HNode::makeRoot(8, "Sailing");
  return Hierarchy(nodes);
}

TEST(HierarchyTest, SailTest) {
  const int len = 18;
  int toParse[len] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 0, 0, 0, 0, 0};
  Hierarchy h = makeMiniSailGrammar();
  std::shared_ptr<HTree> tree = h.parse(Arrayi(len, toParse));

  EXPECT_EQ(tree->index(), 8);
  EXPECT_EQ(tree->childCount(), 3);
  std::shared_ptr<HTree> c = tree->child(1);
  EXPECT_EQ(c->left(), 7);
  EXPECT_EQ(c->right(), 13);
  //tree->disp(&std::cout, h.labels());
}

} /* namespace sail */
