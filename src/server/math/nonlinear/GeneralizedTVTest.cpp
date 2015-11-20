/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>

#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/common/LineKM.h>
#include <random>

using namespace sail;

namespace {
  constexpr int partCount = 100;
  constexpr int sampleCount = 5*partCount;

  double testfun(double x) {
    int a = 2*partCount;
    int b = a + partCount;
    if (x < a) {
      return 0;
    } else if (x < b) {
      return LineKM(a, b, 0, 1)(x);
    }
    return 1;
  }

  Arrayd makeGT() {
    return Arrayd::fill(sampleCount, [](int i) {return testfun(i);});
  }

  Arrayd addNoise(Arrayd Y, double noise, std::default_random_engine &engine) {
    std::uniform_real_distribution<double> distrib(-noise, noise);
    return toArray(map([&](double x) {return x + distrib(engine);}, Y));
  }
}

TEST(GeneralizedTVTest, Test) {

  std::default_random_engine engine(0);
  Arrayd gt = makeGT();
  Arrayd noisy = addNoise(gt, 0.5, engine);

  GeneralizedTV tv;
  UniformSamplesd filtered = tv.filter(noisy, 2, 60);
  Arrayd X = filtered.makeCenteredX();
  Arrayd F = filtered.interpolateLinear(X);

  double sumNoisyErrors = 0;
  double sumFilteredErrors = 0;
  for (int i = 0; i < sampleCount; i++) {
    sumNoisyErrors += std::abs(gt[i] - noisy[i]);
    sumFilteredErrors += std::abs(gt[i] - F[i]);
  }
  EXPECT_LT(10*sumFilteredErrors, sumNoisyErrors);
}


