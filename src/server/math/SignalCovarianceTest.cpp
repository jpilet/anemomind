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


  SignalData<double> xData(time, X, s);
  SignalData<double> x2Data(time, X2, s);
  EXPECT_NEAR(2.0*xData.standardDeviation(), x2Data.standardDeviation(), 1.0e-6);
  //auto stdX = slidingWindowStandardDeviation(time, X, X2)
}

TEST(SignalCovariance, Test1) {
  EXPECT_NEAR(smoothNonNegAbs(3.0, 2.0), 3.0, 1.0e-6);
  EXPECT_NEAR(smoothNonNegAbs(-3.0, 2.0), 3.0, 1.0e-6);
  EXPECT_NEAR(smoothNonNegAbs(-1.0, 2.0), smoothNonNegAbs(1.0, 2.0), 1.0e-6);
  EXPECT_NEAR(smoothNonNegAbs(1.999999999, 2.0), smoothNonNegAbs(2.0000001, 2.0), 1.0e-4);
  EXPECT_LT(0.5, smoothNonNegAbs(0.0, 2.0));
}
