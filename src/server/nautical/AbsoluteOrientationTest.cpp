/*
 * AbsoluteOrientationTest.cpp
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#include <server/nautical/AbsoluteOrientation.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(AbsoluteOrientationTest, MapObjectValuesTest) {
  TypedAbsoluteOrientation<double> orient0{
    45.0_deg, 60.0_deg, 74.0_deg
  };

  TypedAbsoluteOrientation<float> orient1 = orient0.mapObjectValues(
      [](double x) {return static_cast<float>(x);});

  EXPECT_NEAR(orient0.heading.degrees(), orient1.heading.degrees(), 1.0e-5);
  EXPECT_NEAR(orient0.roll.degrees(), orient1.roll.degrees(), 1.0e-5);
  EXPECT_NEAR(orient0.pitch.degrees(), orient1.pitch.degrees(), 1.0e-5);
}



