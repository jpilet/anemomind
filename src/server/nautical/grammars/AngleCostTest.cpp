/*
 *  Created on: Jun 30, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/AngleCost.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(AngleCost, BasicTest) {
  Angle<double> a = Angle<double>::radians(0);
  Angle<double> b = Angle<double>::radians(0.5*M_PI);
  AngleCost cost;
  cost.add(0, a);
  cost.add(1, b);
  EXPECT_NEAR(cost.calcCost(0, a), 0.0, 1.0e-6);
  EXPECT_NEAR(cost.calcCost(0, b), 0.5, 1.0e-6);
  EXPECT_NEAR(cost.calcCost(0, -b), 0.5, 1.0e-6);
  EXPECT_NEAR(cost.calcCost(0, -b + Angle<double>::radians(4.0*M_PI)), 0.5, 1.0e-6);
}


