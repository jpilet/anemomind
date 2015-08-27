/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SparsityConstrained.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(SparsityConstrained, DistributeWeights) {
  Arrayd R0{1.0, 1.0e-9};
  Arrayd W0 = SparsityConstrained::distributeWeights(R0, 9.9);
  EXPECT_EQ(W0.size(), 2);
}


