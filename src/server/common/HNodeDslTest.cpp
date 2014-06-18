/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/HNodeDsl.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(HNodeDslTest, BasicDsl) {
  HNodeGroup g = HNodeGroup(8, "top",
                  HNodeGroup(6, "starboard-tack",
                      HNodeGroup(0, "close-hauled") +
                      HNodeGroup(1, "beam-reach") +
                      HNodeGroup(2, "broad-reach")
                  ) +
                  HNodeGroup(7, "port-tack",
                      HNodeGroup(3, "close-hauled") +
                      HNodeGroup(4, "beam-reach") +
                      HNodeGroup(5, "broad-reach")
                  )
                );
  Array<HNode> nodes = g.compile("simple-grammar-%03d");
  EXPECT_EQ(nodes.size(), 9);
  for (int i = 0; i < 9; i++) {
    EXPECT_EQ(nodes[i].index(), i);
  }
  EXPECT_EQ(nodes[7].parent(), 8);
  EXPECT_EQ(nodes[7].description(), "port-tack");
}

TEST(HNodeDslTest, BasicDslImplicit) {
  HNodeGroup g = HNodeGroup("top",
                  HNodeGroup("starboard-tack",
                      // Still a good idea to give explicit indices to leaf nodes because
                      // they are terminal symbols also used by the StateAssign object.
                      HNodeGroup(0, "close-hauled") +
                      HNodeGroup(1, "beam-reach") +
                      HNodeGroup(2, "broad-reach")
                  ) +
                  HNodeGroup("port-tack",
                      HNodeGroup(3, "close-hauled") +
                      HNodeGroup(4, "beam-reach") +
                      HNodeGroup(5, "broad-reach")
                  )
                );
  Array<HNode> nodes = g.compile("simple-grammar-%03d");
  EXPECT_EQ(nodes.size(), 9);
  for (int i = 0; i < 9; i++) {
    EXPECT_EQ(nodes[i].index(), i);
  }

  // Assuming a preorder traversal internally.
  EXPECT_EQ(nodes[6].parent(), -1);
  EXPECT_EQ(nodes[6].description(), "top");
}


