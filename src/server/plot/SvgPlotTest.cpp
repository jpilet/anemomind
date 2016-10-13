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
  auto c0 = getCorner(box, 0);
  EXPECT_NEAR((c0 - Eigen::Vector4d(0, 0, 0, 1)).norm(), 0.0, 1.0e-6);

  //BBox3d projectBBox(const Eigen::Matrix4d &pose, const BBox3d &box);
  //Eigen::Matrix<double, 2, 4> computeTotalProjection(
  //const BBox3d &a, const Settings2d &settings);
}
