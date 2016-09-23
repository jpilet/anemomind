/*
 * FitnessTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/calib/Fitness.h>

using namespace sail;


/*struct ServerBoatStateSettings {
  static const bool withBoatOverGround = false;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = false;
  static const bool withPitch = false;
  static const bool withIMU = false;
};*/
struct TestSettings1 {
  static const bool withBoatOverGround = false;
  static const bool withCurrentOverGround = false;
  static const bool withHeel = false;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings1>::valueDimension ==
        0 + 2 + 0 + 2 + 0 + 0, "Failed");


TEST(FitnessTest, VectorizationTest) {
  double values[2] = {4.5, 5.6};
  const double *src = values;
  HorizontalMotion<double> x =
      TypeVectorizer<double, HorizontalMotion<double> >::read(&src);
  EXPECT_EQ(src, values + 2);
  double values2[2] = {0.0, 0.0};
  double *dst = values2;
  TypeVectorizer<double, HorizontalMotion<double> >::write(x, &dst);
  EXPECT_NEAR(values[0], values2[0], 1.0e-6);
  EXPECT_NEAR(values[1], values2[1], 1.0e-6);
}
