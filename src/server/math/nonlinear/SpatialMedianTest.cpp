/*
 * SpatialMedianTest.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/SpatialMedian.h>

using namespace sail;

namespace {
  Eigen::Matrix<double, 1, 1> pt(double x) {
    Eigen::Matrix<double, 1, 1> dst;
    dst(0) = x;
    return dst;
  }

  Eigen::Matrix<double, 2, 1> pt2(double x) {
    Eigen::Matrix<double, 2, 1> dst;
    dst(0) = x;
    dst(1) = 0.2*x;
    return dst;
  }
}

TEST(SpatialMedianTest, OneDimensional) {
  Array<Eigen::Matrix<double, 1, 1> > pts{
    pt(-300), pt(1), pt(2), pt(5), pt(17)
  };

  SpatialMedian::Settings settings;
  auto median = SpatialMedian::compute(pts, settings);

  EXPECT_NEAR(median(0), 2.0, 1.0e-3);
}

TEST(SpatialMedianTest, TwoDimensionalColinear) {
  Array<Eigen::Matrix<double, 2, 1> > pts{
    pt2(-300), pt2(1), pt2(2), pt2(5), pt2(17)
  };

  SpatialMedian::Settings settings;
  auto median = SpatialMedian::compute(pts, settings);

  EXPECT_NEAR(median(0), 2.0, 1.0e-3);
  EXPECT_NEAR(median(1), 0.2*2.0, 1.0e-3);
}


