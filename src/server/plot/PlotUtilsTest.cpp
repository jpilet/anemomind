/*
 * PlotUtilsTest.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */
#include <gtest/gtest.h>
#include <server/plot/PlotUtils.h>
#include <server/common/Env.h>

using namespace sail;
using namespace sail::PlotUtils;

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

Eigen::Vector3d getLower(const BBox3d &x) {
  return Eigen::Vector3d(
      x.getSpan(0).minv(),
      x.getSpan(1).minv(),
      x.getSpan(2).minv());
}

Eigen::Vector3d getUpper(const BBox3d &x) {
  return Eigen::Vector3d(
      x.getSpan(0).maxv(),
      x.getSpan(1).maxv(),
      x.getSpan(2).maxv());
}

TEST(PlotUtilsTest, Geometry) {
  EXPECT_TRUE(isEmpty(makeEmptyBBox()));
  auto box = makeBox123();


  //Eigen::Vector4d getCorner(const BBox3d &box, int cornerIndex0);
  EXPECT_NEAR((getCorner(box, 0) - Eigen::Vector4d(0, 0, 0, 1)).norm(), 0.0, 1.0e-6);
  EXPECT_NEAR((getCorner(box, 1) - Eigen::Vector4d(1, 0, 0, 1)).norm(), 0.0, 1.0e-6);
  EXPECT_NEAR((getCorner(box, 7) - Eigen::Vector4d(1, 2, 3, 1)).norm(), 0.0, 1.0e-6);

  Eigen::Matrix4d pose = Eigen::Matrix<double, 4, 4>::Identity();
  pose(1, 1) = -1.0;
  pose(2, 2) = -1.0;

  auto projected = projectBBox(pose, box);

  EXPECT_NEAR((getLower(projected) - Eigen::Vector3d(0, -2 , -3)).norm(),
      0.0, 1.0e-6);
  EXPECT_NEAR((getUpper(projected) - Eigen::Vector3d(1, 0, 0)).norm(),
      0.0, 1.0e-6);
}
