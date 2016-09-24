/*
 * AbsoluteOrientationTest.cpp
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#include <server/nautical/AbsoluteOrientation.h>
#include <gtest/gtest.h>
#include <random>
#include <server/math/EigenUtils.h>

using namespace sail;

TEST(AbsoluteOrientationTest, MapObjectValuesTest) {
  TypedAbsoluteOrientation<double, OrientCoding::BNO055> orient0{
    45.0_deg, 60.0_deg, 74.0_deg
  };

  TypedAbsoluteOrientation<float, OrientCoding::BNO055> orient1 = orient0.mapObjectValues(
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

TEST(BoatStateTest, OrientationOrthonormality) {
  std::default_random_engine rng;
  std::uniform_real_distribution<double>
    distrib(0.0, 2.0*M_PI);
  auto randomAngle = [&]() {
    auto value = Angle<double>::radians(distrib(rng));
    return value;
  };

  for (int i = 0; i < 30; i++) {
    AbsoluteBoatOrientation<double> o{
      randomAngle(), randomAngle(), randomAngle()
    };

    Eigen::Matrix<double, 3, 3> R = orientationToMatrix(o);
    Eigen::Matrix3d RtR = R.transpose()*R;

    EXPECT_TRUE(
        (isZero<double, 3, 3>(
            RtR - Eigen::Matrix3d::Identity(),
            1.0e-5)));
  }
}

TEST(BoatStateTest, ElementaryOrientations) {
  double k = 1.0/sqrt(2.0);
  { // heading
    AbsoluteBoatOrientation<double> o{45.0_deg, 0.0_deg, 0.0_deg};
    auto R = orientationToMatrix(o);
    EXPECT_LT(
        (R*Eigen::Vector3d(1, 0, 0)
           - Eigen::Vector3d(k, -k, 0.0)).norm(), 1.0e-6);
    EXPECT_LT(
        (R*Eigen::Vector3d(0, 1, 0)
           - Eigen::Vector3d(k, k, 0.0)).norm(), 1.0e-6);
    EXPECT_LT(
        (R*Eigen::Vector3d(0, 0, 1)
           - Eigen::Vector3d(0, 0, 1)).norm(), 1.0e-6);
  }{ // roll
    AbsoluteBoatOrientation<double> o{0.0_deg, 45.0_deg, 0.0_deg};
    auto R = orientationToMatrix(o);
    EXPECT_LT(
        (R*Eigen::Vector3d(1, 0, 0)
           - Eigen::Vector3d(k, 0, -k)).norm(), 1.0e-6);
    EXPECT_LT(
        (R*Eigen::Vector3d(0, 1, 0)
           - Eigen::Vector3d(0, 1, 0)).norm(), 1.0e-6);
    EXPECT_LT(
        (R*Eigen::Vector3d(0, 0, 1)
           - Eigen::Vector3d(k, 0, k)).norm(), 1.0e-6);
  }{ // pitch
    AbsoluteBoatOrientation<double> o{0.0_deg, 0.0_deg, 45.0_deg};
    auto R = orientationToMatrix(o);
    EXPECT_LT(
        (R*Eigen::Vector3d(1, 0, 0)
           - Eigen::Vector3d(1, 0, 0)).norm(), 1.0e-6);
    EXPECT_LT(
        (R*Eigen::Vector3d(0, 1, 0)
           - Eigen::Vector3d(0, k, k)).norm(), 1.0e-6);
    EXPECT_LT(
        (R*Eigen::Vector3d(0, 0, 1)
           - Eigen::Vector3d(0, -k, k)).norm(), 1.0e-6);
  }
}


