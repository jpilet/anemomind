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
  bool orthonormal = true;
  BBox3d defaultBBox = makeDefaultBBox();
  double pixelsPerUnit = 100.0;
  bool dataOnTop = false;
};

bool isEmpty(const BBox3d &box);

// If the bbox is too small, get the default one.
BBox3d ensureGoodBBox(
    const BBox3d &bbox,
    const Settings2d &settings);

// Assuming that we enumerate the eight corners of a
// bounding box, this function will return one of
// those corners as homogeneous coordinates.
Eigen::Vector4d getCorner(const BBox3d &box, int cornerIndex0);

// Compute the bounding box of applying a transformation
// to all the corners of a bounding box.
BBox3d projectBBox(const Eigen::Matrix4d &pose, const BBox3d &box);

// Given a bounding box of the objects we want to draw
// this function computes a projection matrix, that
// when applied to the objects, will neatly fit the objects
// into the device space.
Eigen::Matrix<double, 2, 4> computeTotalProjection(
    const BBox3d &a, const Settings2d &settings);

struct RGB {
  RGB(double r, double g, double b) : red(r), green(g), blue(b) {}
  RGB() {}

  double red = 0;
  double green = 0;
  double blue = 0;

  static RGB black() {
    return RGB(0, 0, 0);
  }
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
