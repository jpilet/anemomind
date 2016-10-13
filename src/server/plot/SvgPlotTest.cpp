/*
 * SvgPlotTest.cpp
 *
 *  Created on: 13 Oct 2016
 *      Author: jonas
 */

#include <server/plot/SvgPlot.h>
#include <gtest/gtest.h>

using namespace sail;
using namespace sail::SvgPlot;

BBox3d makeEmptyBBox() {
  BBox3d b;
  double a[3] = {0, 0, 0};
  b.extend(a);
  return b;
}

BBox3d makeBox123() {
  double a[3] = {0, 0, 0};
  double b[3] = {1, 2, 3};
  BBox3d box;
  box.extend(a);
  box.extend(b);
  return box;
}

TEST(SvgPlotTest, Geometry) {
  EXPECT_TRUE(isEmpty(makeEmptyBBox()));
  auto box = makeBox123();


  //Eigen::Vector4d getCorner(const BBox3d &box, int cornerIndex0);
  EXPECT_NEAR((getCorner(box, 0) - Eigen::Vector4d(0, 0, 0, 1)).norm(), 0.0, 1.0e-6);
  EXPECT_NEAR((getCorner(box, 1) - Eigen::Vector4d(1, 0, 0, 1)).norm(), 0.0, 1.0e-6);
  EXPECT_NEAR((getCorner(box, 7) - Eigen::Vector4d(1, 2, 3, 1)).norm(), 0.0, 1.0e-6);

  Eigen::Matrix4d pose = Eigen::Matrix<double, 4, 4>::Identity();
  pose(1, 1) = -1.0;
  pose(2, 3) = -1.0;

  auto projected = projectBBox(pose, box);


  //Eigen::Matrix<double, 2, 4> computeTotalProjection(
  //const BBox3d &a, const Settings2d &settings);
}
