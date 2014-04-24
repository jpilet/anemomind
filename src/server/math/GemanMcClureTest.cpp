/*
 *  Created on: 2014-04-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <gtest/gtest.h>
#include <server/math/GemanMcClure.h>

using namespace sail;

TEST(GemanMcClureTest, WeightTest) {
  GemanMcClure gmc(10.0, 10.0, 1, 1);
  EXPECT_NEAR(gmc.getWeight(0), 0.5, 1.0e-6);
  gmc.addResiduals(Arrayd::args(10.0));
  EXPECT_NEAR(gmc.getWeight(0), 0.5, 1.0e-6);
  gmc.addResiduals(Arrayd::args(10000.1));
  EXPECT_LE(gmc.getWeight(0), 0.4);
}


