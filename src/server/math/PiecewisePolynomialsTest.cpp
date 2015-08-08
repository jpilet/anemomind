/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/PiecewisePolynomials.h>

using namespace sail;

TEST(PiecewisePolynomialsTest, Test0) {
  int count = 300;
  Arrayd X(count);
  Arrayd Y(count);
  LineKM sampleToX(0, count-1, -1, 1);
  for (int i = 0; i < count; i++) {
    double x = sampleToX(i);
    X[i] = x;
    Y[i] = (x < 0? -1 : 1) + 0.1*(sin(300*x) + cos(34*x + 34234));
  }
  auto pieces = PiecewisePolynomials::optimize<1>(X, Y, count, sampleToX, 2);
  EXPECT_EQ(pieces.size(), 2);
}


