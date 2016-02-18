/*
 *  Created on: 2014-10-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/UniformSamples.h>

using namespace sail;

TEST(UniformSamplesd, InterpolationTest) {
  Arrayd samples = Arrayd{1.0, 1.5};
  LineKM sampling = LineKM::identity();

  UniformSamplesd x(sampling, samples);
  EXPECT_NEAR(x.interpolateLinear(0.25), 1.125, 1.0e-6);
  EXPECT_NEAR(x.interpolateLinearDerivative(0.25), 0.5, 1.0e-6);
  EXPECT_NEAR(x.interpolateLinearDerivative(0.78), 0.5, 1.0e-6);
}


