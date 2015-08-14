/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/SignalCovariance.h>

using namespace sail;
using namespace sail::SignalCovariance;

TEST(SignalCovariance, Test0) {
  Arrayd time{0, 1, 2, 3, 4};
  Arrayd X{0, 0, 1, 1, 1};
  Arrayd X2{0, 0, 2, 2, 2};
  Arrayd W{0, 0, 0, 1, 1};
  Arrayd Y{0, 1, 1, 0, 0};
  Arrayd Z{1, 1.01, 1, 1.01, 1};
  auto weight = 1.0/4;

  auto xstd = sqrt(4*sqr(0.5)/weight);
  auto ystd = sqrt(4*sqr(0.5)/weight);
  auto normalizationXX = 1.0/(xstd*xstd);

  Settings s;
  s.windowSize = 4;
  s.maxResidualCount = 1;

  Arrayd xx = slidingWindowCovariances(time, X, X, s);
  auto sumCovXX = 4*sqr(0.5)*normalizationXX;

  EXPECT_EQ(xx.size(), 1);
  EXPECT_NEAR(xx[0], sumCovXX, 1.0e-5);

  // Due to normalization, doubling one of the signals should not change much.
  Arrayd xx2 = slidingWindowCovariances(time, X, X2, s);
  EXPECT_NEAR(xx2[0], sumCovXX, 1.0e-5);

  // Try with a signal that where we expect less covariance.
  Arrayd xw = slidingWindowCovariances(time, X, W, s);
  EXPECT_LT(xw[0], xx[0]);
  // But not negligible covariance:
  EXPECT_LT(0.01, xw[0]);

  // No covariance
  Arrayd xy = slidingWindowCovariances(time, X, Y, s);
  EXPECT_NEAR(xy[0], sqrt(1.0e-6), 1.0e-5);

  // No covariance.
  Arrayd xz = slidingWindowCovariances(time, X, Z, s);
  EXPECT_NEAR(xz[0], 0.0, 1.0e-3);
}
