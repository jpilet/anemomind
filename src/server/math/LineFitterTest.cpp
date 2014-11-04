/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/LineFitter.h>
#include <gtest/gtest.h>
#include <random>
#include <server/common/string.h>

using namespace sail;

TEST(LineFitterTest, NoisyStep) {
  const int dataCount = 300;
  Arrayd X(dataCount);
  Arrayd Y(dataCount);
  std::default_random_engine e(0);
  double marg = 1.0e-3;
  std::uniform_real_distribution<double> xvalue(0 + marg, 1 - marg);
  std::uniform_real_distribution<double> noise(-0.1, 0.1);
  for (int i = 0; i < dataCount; i++) {
    X[i] = xvalue(e);
    Y[i] = (X[i] < 0.5? 0 : 1) + noise(e);
  }

  int sampleCount = 31;
  LineKM sampling(0, sampleCount-1, 0.0, 1.0);

  LineFitter fitter(1.0e6, 1);
  Array<LineFitter::LineSegment> segments = fitter.optimize(sampling, sampleCount, X, Y);
  EXPECT_EQ(segments.size(), 2);
  EXPECT_EQ(segments[0].span(), Spani(0, 15));
  EXPECT_EQ(segments[1].span(), Spani(15, 30));
}



