/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/UniformSamples.h>

using namespace sail;

TEST(UniformSamples, InterpolationTest) {
  Arrayd samples = Arrayd::args(1.0, 1.5);
  LineKM sampling(0.0, 1.0);

  UniformSamples x(sampling, samples);
  EXPECT_NEAR(x.interpolateLinear(0.25), 1.25, 1.0e-6);
  EXPECT_NEAR(x.interpolateLinearDerivative(0.25), 0.5, 1.0e-6);
  EXPECT_NEAR(x.interpolateLinearDerivative(0.78), 0.5, 1.0e-6);
}


