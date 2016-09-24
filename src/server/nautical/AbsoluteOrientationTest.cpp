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

TEST(AbsoluteOrientationTest, RotationMatrix) {
  AbsoluteOrientation orient;



  Eigen::Matrix4d expected; // Copy/pasted from the orientdemo/demo.js
  expected << 0.6254621744155884,-0.2321031540632248,0.744933009147644,0,
              0.4090060591697693,0.9105769395828247,-0.05969670042395592,0,
              -0.6644629836082458,0.3420201539993286,0.6644629836082458,0,
              0,0,0,1;


  orient.heading = 45.0_deg;
  orient.pitch = 20.0_deg;
  orient.roll = -14.3_deg;
  auto R = BNO055AnglesToRotation(orient);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_NEAR(R(i, j), expected(i, j), 1.0e-5);
    }
  }
}


