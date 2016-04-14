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
    Y[i] = (i < 200? -12 : 3) + 0.1*(sin(300*x) + cos(34*x + 34234));
  }
  auto pieces = PiecewisePolynomials::optimizeForSegmentCount<1>(
      X, Y, count, sampleToX, 2);
  EXPECT_EQ(pieces.size(), 2);
  EXPECT_EQ(pieces[0].span.minv(), 0);
  EXPECT_NEAR(pieces[0].span.maxv(), 200, 3);
  EXPECT_EQ(pieces[0].span.maxv(), pieces[1].span.minv());
  EXPECT_EQ(pieces[1].span.maxv(), count-1);
  EXPECT_NEAR(pieces[0].constantValue(), -12, 0.01);
  EXPECT_NEAR(pieces[1].constantValue(), 3, 0.01);
}

TEST(PiecewisePolynomialsTest, Sparse) {

  // Like the previous test, but make
  // sure that it doesn't break to due illposedness
  // when not having as many samples.

  int sampleCount = 300;
  int step = 4;
  int dataCount = sampleCount/step;
  Arrayd X(dataCount);
  Arrayd Y(dataCount);
  LineKM sampleToX(0, sampleCount-1, -1, 1);
  for (int i = 0; i < dataCount; i++) {
    auto k = i*step;
    double x = sampleToX(k);
    X[i] = x;
    Y[i] = (k < 200? -12 : 3) + 0.1*(sin(300*x) + cos(34*x + 34234));
  }
  auto pieces = PiecewisePolynomials::optimizeForSegmentCount<1>(X, Y, sampleCount, sampleToX, 2);
  EXPECT_EQ(pieces.size(), 2);
  EXPECT_EQ(pieces[0].span.minv(), 0);
  EXPECT_NEAR(pieces[0].span.maxv(), 200, 3);
  EXPECT_EQ(pieces[0].span.maxv(), pieces[1].span.minv());
  EXPECT_EQ(pieces[1].span.maxv(), sampleCount-1);
  EXPECT_NEAR(pieces[0].constantValue(), -12, 0.01);
  EXPECT_NEAR(pieces[1].constantValue(), 3, 0.01);
}


