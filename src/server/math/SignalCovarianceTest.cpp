/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/SignalCovariance.h>
#include <server/common/Span.h>

using namespace sail;
using namespace sail::SignalCovariance;

// Make sure that the value of a residual is reasonable
TEST(SignalCovariance, Test0) {

  // The time of each sample.
  Arrayd time{0, 1, 2, 3, 4};

  // The signals. Only the first four values are used.
  // However, we use the last time sample in the time signal
  // to determine the weight.
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

  EXPECT_EQ(xx.size(), 1);

  // Due to normalization, doubling one of the signals should not change much.
  Arrayd xx2 = slidingWindowCovariances(time, X, X2, s);

  // Try with a signal that where we expect less covariance.
  Arrayd xw = slidingWindowCovariances(time, X, W, s);
  EXPECT_LT(xw[0], xx[0]);
  // But not negligible covariance:
  EXPECT_LT(0.01, xw[0]);

  // No covariance. It will not be exactly 0 due to the positive number added to ensure the square
  // root is not applied close to 0.
  Arrayd xy = slidingWindowCovariances(time, X, Y, s);
  EXPECT_NEAR(xy[0], 0.0, 1.0e-3);

  // No covariance.
  Arrayd xz = slidingWindowCovariances(time, X, Z, s);
  EXPECT_NEAR(xz[0], 0.0, 1.0e-3);

  SignalData<double> xData(time, X, s);
  SignalData<double> x2Data(time, X2, s);
  EXPECT_NEAR(2.0*xData.standardDeviation(), x2Data.standardDeviation(), 1.0e-6);
  //auto stdX = slidingWindowStandardDeviation(time, X, X2)
}


// Make sure that the result has the right length
TEST(SignalCovariance, Lengths) {
  Settings s;
  s.windowSize = 4;
  s.maxResidualCount = 3;

  auto T = Spani(0, 8).map<double>([&](int i) {return double(i);});
  auto X = Arrayd::fill(6, 1);
  auto Y = Arrayd::fill(8, 1);
  auto Xres = slidingWindowCovariances(T.sliceTo(6), X, X, s);
  auto Yres = slidingWindowCovariances(T, Y, Y, s);
  EXPECT_EQ(Xres.size(), 2);
  EXPECT_EQ(Yres.size(), 3);
}
