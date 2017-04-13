/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/MeanAndVar.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(MeanAndVarTest, SimpleTest) {
  MeanAndVar a;
  a.add(1);
  EXPECT_NEAR(a.mean(), 1.0, 1.0e-6);
  a.add(2.0);
  EXPECT_NEAR(a.variance(), 0.5, 1.0e-6);
  MeanAndVar c;
  c.add(3);
  MeanAndVar b = a + c;
  EXPECT_NEAR(b.mean(), 2.0, 1.0e-6);
  EXPECT_NEAR(b.variance(), 1.0, 1.0e-6);
}

TEST(MeanAndVarTest, Array) {
  Arrayd X = Arrayd{1, 2, 5};
  MeanAndVar x(X);
  EXPECT_NEAR(x.mean(), 2.667, 0.01);
  EXPECT_NEAR(x.variance(), 4.3333, 0.01);
  EXPECT_NEAR(x.standardDeviation(), 2.0817, 0.01);
}

TEST(MeanAndVarTest, MinMax) {
  Arrayd X = Arrayd{-7, 1, 2, 5};
  MeanAndVar x(X);
  EXPECT_EQ(-7, x.min());
  EXPECT_EQ(5, x.max());

  MeanAndVar null;
  x = x + null;
  EXPECT_EQ(-7, x.min());
  EXPECT_EQ(5, x.max());

  MeanAndVar bigger;
  bigger.add(18);
  EXPECT_EQ(18, bigger.max());
  x = x + bigger;
  EXPECT_EQ(18, x.max());
}

