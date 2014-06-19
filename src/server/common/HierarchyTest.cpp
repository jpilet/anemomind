/*
 *  Created on: 2014-01-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Hierarchy.h"
#include "gtest/gtest.h"
#include <server/common/HNodeGroup.h>

namespace sail {

namespace {
Array<HNode> makeBinaryTestNodes() {
  return HNodeGroup(2, "BinaryDigits",
      HNodeGroup(0, "Zero") + HNodeGroup(1, "One")
  ).compile("bin-%03d");
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
  HInner *i4 = new HInner(2, p0);
  std::shared_ptr<HTree> p4(i4);
  i4->add(p1);
  i4->add(p2);
  EXPECT_TRUE(tree->child(0)->equals(p0));
  EXPECT_TRUE(tree->child(1)->equals(p1));
  EXPECT_TRUE(tree->child(2)->equals(p2));
  EXPECT_TRUE(p4->equals(tree));
  EXPECT_FALSE(p4->equals(p0));
}

namespace {
Array<HNode> reorder3(Array<HNode> nodes) {
  const int count = 3;
  int newPos[count] = {2, 0, 1};
  Array<HNode> dst(count);
  for (int i = 0; i < count; i++) {
    dst[newPos[i]] = nodes[i];
  }
  return dst;
}
}

TEST(HierarchyTest, ParseTestBinaryReordered) {
  const int len = 8;
  int stringToParse[len] = {0, 0, 0, 0, 1, 1, 1, 0};
  Hierarchy h(reorder3(makeBinaryTestNodes()));
  std::shared_ptr<HTree> tree = h.parse(Arrayi(len, stringToParse));
  EXPECT_EQ(tree->left(), 0);
  EXPECT_EQ(tree->right(), len);
  EXPECT_EQ(tree->childCount(), 3);

  //tree->disp(&std::cout);
  std::shared_ptr<HTree> p0(new HLeaves(0, 0, 4));
  std::shared_ptr<HTree> p1(new HLeaves(4, 1, 3));
  std::shared_ptr<HTree> p2(new HLeaves(7, 0, 1));
  HInner *i4 = new HInner(2, p0);
  std::shared_ptr<HTree> p4(i4);
  i4->add(p1);
  i4->add(p2);
  EXPECT_TRUE(tree->child(0)->equals(p0));
  EXPECT_TRUE(tree->child(1)->equals(p1));
  EXPECT_TRUE(tree->child(2)->equals(p2));
  EXPECT_TRUE(p4->equals(tree));
  EXPECT_FALSE(p4->equals(p0));
}

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
  ).compile("test-%03d");
}


TEST(HierarchyTest, SailTest) {
  const int len = 18;


  // Graphical explanation of the parsing
  // Depth 0:        [-------------------------8--------------------------]
  // Depth 1:        [----------6--------][--------7-------][------6------]
  // Depth 2:        [-0--][--1-][---2---][-3--][--4-][-5--][------0------]
  int toParse[len] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 0, 0, 0, 0, 0};


    Hierarchy h = makeMiniSailGrammar();
    std::shared_ptr<HTree> tree = h.parse(Arrayi(len, toParse));

    EXPECT_EQ(tree->index(), 8);
    EXPECT_EQ(tree->childCount(), 3);
    std::shared_ptr<HTree> c = tree->child(1);
    EXPECT_EQ(c->left(), 7);
    EXPECT_EQ(c->right(), 13);
}

Hierarchy makeMiniSailGrammar2() {
  return HNodeGroup(4, "On the sea",
      HNodeGroup(3, "Sailing",
          HNodeGroup(1, "Starboard tack") + HNodeGroup(2, "Port tack")
      )
      +
      HNodeGroup(0, "In irons")
  ).compile("test-%03d");
}

// A grammar with terminals at different depths
TEST(HierarchyTest, SailTestMultiDepth) {
  const int len = 11;

  // Graphical explanation
  // Depth 0:        [--------------4----------------]
  // Depth 1:        [--3-][---0---][-------3--------]
  // Depth 2:        [--1-]         [---2---][---1---]

  int toParse[len] = {1, 1, 0, 0, 0, 2, 2, 2, 1, 1, 1};
  Hierarchy h = makeMiniSailGrammar2();
  std::shared_ptr<HTree> t = h.parse(Arrayi(len, toParse));
  EXPECT_EQ(t->childCount(), 3);
  EXPECT_EQ(t->child(1)->index(), 0);
  EXPECT_EQ(t->child(1)->childCount(), 0);
  EXPECT_EQ(t->child(0)->childCount(), 1);
  EXPECT_EQ(t->child(0)->child(0)->index(), 1);
}


} /* namespace sail */
