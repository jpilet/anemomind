/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>

#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/common/RandomEngine.h>
#include <server/common/LineKM.h>

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

  Arrayd addNoise(Arrayd Y, double noise, RandomEngine::EngineType &engine) {
    std::uniform_real_distribution<double> distrib(-noise, noise);
    return Y.map<double>([&](double x) {return x + distrib(engine);});
  }
}

TEST(GeneralizedTVTest, Test) {
  RandomEngine::EngineType engine = RandomEngine::EngineType();
  engine.seed(0);
  Arrayd gt = makeGT();
  Arrayd noisy = addNoise(gt, 0.5, engine);
}


