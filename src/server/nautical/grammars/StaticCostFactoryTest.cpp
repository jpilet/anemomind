/*
 *  Created on: Jun 20, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/HNodeGroup.h>
#include <server/nautical/grammars/StaticCostFactory.h>


using namespace sail;

namespace {
  Hierarchy makeMiniGrammar() {
    return HNodeGroup(4, "Symbols",
        HNodeGroup(3, "Digits",
            HNodeGroup(0, "0") + HNodeGroup(1, "1")
        )
        +
        HNodeGroup(2, ".")
        ).compile("mini-grammar-%03d");
  }

}

TEST(StaticCostFactoryTest, Full) {
  StaticCostFactory f(makeMiniGrammar());
  {
    MDArray2b con = f.connections();
    MDArray2d costs = f.staticTransitionCosts();
    EXPECT_EQ(con.rows(), 3);
    EXPECT_EQ(con.cols(), 3);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        EXPECT_FALSE(con(i, j));
        EXPECT_EQ(costs(i, j), 0.0);
      }
    }
  }
  f.connect(4, 4, 1.9);
  {
    MDArray2b con = f.connections();
    MDArray2d costs = f.staticTransitionCosts();
    EXPECT_EQ(con.rows(), 3);
    EXPECT_EQ(con.cols(), 3);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        EXPECT_TRUE(con(i, j));
        EXPECT_EQ(costs(i, j), 1.9);
      }
    }
  }
  f.connect(4, 4, [=](int i, int j) {return -1.0 + (i == j? 0.1 : -0.9);});
  {
    MDArray2d costs = f.staticTransitionCosts();
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        EXPECT_NEAR(costs(i, j), (i == j? 1.0 : 0.0), 1.0e-6);
      }
    }
  }
}

TEST(StaticCostFactoryTest, StateCosts) {
  StaticCostFactory f(makeMiniGrammar());
  f.addStateCost(4, [=](int i) {return i;});
  {
    Arrayd costs = f.staticStateCosts();
    for (int i = 0; i < costs.size(); i++) {
      EXPECT_NEAR(costs[i], i, 1.0e-6);
    }
  }
  f.addStateCost(4, [=](int i) {return 2 - i;});
  {
    Arrayd costs = f.staticStateCosts();
    for (int i = 0; i < costs.size(); i++) {
      EXPECT_NEAR(costs[i], 2, 1.0e-6);
    }
  }
}

TEST(StaticCostFactoryTest, AsymCon) {
  StaticCostFactory f(makeMiniGrammar());
  f.connect(3, 2, 0.0);
  MDArray2b con = f.connections();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ((j == 2) && (i == 0 || i == 1), con(i, j));
    }
  }
}

TEST(StaticCostFactoryTest, SymCon) {
  StaticCostFactory f(makeMiniGrammar());
  f.connect(3, 2, 0.0, true);
  MDArray2b con = f.connections();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_EQ(
          ((j == 2) && (i == 0 || i == 1))
            ||
          ((i == 2) && (j == 0 || j == 1)),
         con(i, j));
    }
  }
}




