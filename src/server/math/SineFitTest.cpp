/*
 * SineFitTest.cpp
 *
 *  Created on: 14 Nov 2016
 *      Author: jonas
 */

#include <server/math/SineFit.h>
#include <gtest/gtest.h>

using namespace sail;

void testFor(const Sine &gt) {

  Array<Angle<double>> angles{
    -13.0_deg, 4.7_deg, 83.0_deg, 97.12_deg,
    917.0_deg
  };

  Array<std::pair<Angle<double>, double>> data(angles.size());
  for (int i = 0; i < angles.size(); i++) {
    auto alpha = angles[i];
    data[i] = std::pair<Angle<double>, double>(
        alpha, gt(alpha));
  }

  auto x0 = fit(gt.omega(), data);
  EXPECT_TRUE(x0.defined());
  auto x = x0.get();

  for (auto angle: angles) {
    EXPECT_NEAR(x(angle), gt(angle), 1.0e-6);
  }
}

TEST(SineFit, FittingTests) {
  testFor(Sine(1.0, 1.0, 0.0_deg, 0.0));
  testFor(Sine(4.0, 1.0, 0.0_deg, 0.0));
  testFor(Sine(1.0, 2.0, 0.0_deg, 0.0));
  testFor(Sine(1.0, 9.0, 34.0_deg, 0.0));
  testFor(Sine(1.0, 1.0, 0.0_deg, 17.0));
}


