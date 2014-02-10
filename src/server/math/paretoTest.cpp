#include "pareto.h"
#include <gtest/gtest.h>

using namespace sail;

TEST(ParetoTest, FrontierInsertionTest) {
  ParetoFrontier frontier;
  EXPECT_TRUE(frontier.insert(ParetoElement(1.0, 1.0)));
  EXPECT_EQ(frontier.size(), 1);
  EXPECT_FALSE(frontier.insert(ParetoElement(2, 2)));
  EXPECT_TRUE(frontier.insert(ParetoElement(1.1, 0.9)));
  EXPECT_EQ(frontier.size(), 2);
  EXPECT_FALSE(frontier.insert(ParetoElement(2, 2)));
  EXPECT_TRUE(frontier.insert(ParetoElement(0.0, 0.0)));
  EXPECT_EQ(frontier.size(), 1);
}
