/*
 * PlotUtils.h
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_PLOT_PLOTUTILS_H_
#define SERVER_PLOT_PLOTUTILS_H_

#include <Eigen/Dense>
#include <server/common/Optional.h>
#include <server/common/BBox.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {
namespace PlotUtils {

BBox3d makeDefaultBBox();

struct Settings2d {
  static constexpr double masterMargin = 30;
  double xMargin = masterMargin*1.61803398875; // Golden ratio :-)
  double yMargin = masterMargin;
  double width = 640;
  double height = 480;
  bool axisIJ = false;
  bool orthogonal = true;
  BBox3d defaultBBox = makeDefaultBBox();
};

bool isEmpty(const BBox3d &box);
BBox3d ensureGoodBBox(
    const BBox3d &bbox,
    const Settings2d &settings);
Eigen::Vector4d getCorner(const BBox3d &box, int cornerIndex0);
BBox3d projectBBox(const Eigen::Matrix4d &pose, const BBox3d &box);
Eigen::Matrix<double, 2, 4> computeTotalProjection(
    const BBox3d &a, const Settings2d &settings);

struct RGB {
  double red = 0;
  double green = 0;
  double blue = 0;
};

struct HSV {
  Angle<double> hue = 0.0_deg;
  double saturation = 0.0;
  double value = 0.0;

  static HSV fromHue(Angle<double> h);
};

RGB hsv2rgb(const HSV &hsv);

}
}

#endif /* SERVER_PLOT_PLOTUTILS_H_ */
