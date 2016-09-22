/*
 * FitnessTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/calib/Fitness.h>

using namespace sail;

TEST(FitnessTest, VectorizationTest) {
  double values[2] = {4.5, 5.6};
  const double *src = values;
  HorizontalMotion<double> x =
      TypeVectorizer<double, HorizontalMotion<double> >::read(&src);

}
