/*
 * PlotTest.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/plot/Plot.h>

using namespace sail;

TEST(PlotTest, RenderSettingsDefaults) {
  RenderSettings rs;
  EXPECT_TRUE(Spand(0, 1).contains(rs.rgbColor(0)));
  EXPECT_TRUE(Spand(0, 1).contains(rs.rgbColor(1)));
  EXPECT_TRUE(Spand(0, 1).contains(rs.rgbColor(2)));

  EXPECT_LT(0, rs.lineWidth);
  EXPECT_LT(0, rs.markerSize);
}

namespace {

  class MockPlotModel : public PlotModel {
  public:
    bool wasCalled = false;

    void render(const AbstractArray<Eigen::Vector3d> &pts, const RenderSettings &rs) {
      EXPECT_EQ(rs.lineWidth, 119.0);
      EXPECT_EQ(pts.size(), 2);
      EXPECT_EQ(pts[0], Eigen::Vector3d(1, 2, 3));
      EXPECT_EQ(pts[1], Eigen::Vector3d(4, 5, 8));
      wasCalled = true;
    }
  };

}

TEST(PlotTest, PlottablePoints) {
  RenderSettings rs;
  rs.lineWidth = 119.0;
  PlottablePoints pts(Array<Eigen::Vector3d>{
    Eigen::Vector3d(1, 2, 3),
    Eigen::Vector3d(4, 5, 8)
  }, rs);

  BBox3d expectedBox;
  expectedBox.extend(Eigen::Vector3d(1, 2, 3).data());
  expectedBox.extend(Eigen::Vector3d(4, 5, 8).data());

  EXPECT_EQ(pts.bbox(), expectedBox);

  MockPlotModel pm;
  EXPECT_FALSE(pm.wasCalled);

  pts.render(&pm);

  EXPECT_TRUE(pm.wasCalled);

}
