/*
 * CairoUtils.h
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_PLOT_CAIROUTILS_H_
#define SERVER_PLOT_CAIROUTILS_H_

#include <memory>
#include <cairo/cairo.h>
#include <Eigen/Dense>
#include <server/plot/PlotUtils.h>
#include <random>
#include <server/common/Unmovable.h>

namespace sail {
namespace Cairo {

// Note on line-width, interesting!
// https://www.cairographics.org/tutorial/#L2linewidth
// https://www.cairographics.org/manual/cairo-Transformations.html

std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x);
std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x);

//template <int rows, int cols>
//cairo_matrix_t toCairo(
//    const Eigen::Matrix<double, rows, cols> &mat);
template <int rows, int cols>
cairo_matrix_t toCairo(const Eigen::Matrix<double, rows, cols> &mat);

template <int rows, int cols>
cairo_matrix_t toCairo(const Eigen::Matrix<double, rows, cols> &mat) {
  static_assert(2 <= rows, "Too few rows");
  static_assert(3 <= cols, "Too few cols");
  cairo_matrix_t dst;
  dst.xx = mat(0, 0);
  dst.xy = mat(0, 1);
  dst.yx = mat(1, 0);
  dst.yy = mat(1, 1);
  dst.x0 = mat(0, cols-1);
  dst.y0 = mat(1, cols-1);
  return dst;
}

class WithLocalContext {
public:
  WithLocalContext(cairo_t *cr);
  ~WithLocalContext();
private:
  MAKE_UNMOVABLE(WithLocalContext);
  cairo_t *_cr;
};

// If the current transform from user (x) to device (y) coordinates is
// y = Ax + B,
// then during the life-time of this object, it will be
// y = x + B
// In other words, the user space origin will map to the same point as
// before, but the local scale will be one.
//
// This is useful for drawing things such as small marker, that should be
// invariant to most transformations except translation. Or maybe we want
// the boat drawing to be invariant to zoom.
class WithLocalDeviceScale {
public:
  enum Mode {Determinant, Identity};

  WithLocalDeviceScale(cairo_t *cr, Mode mode);
  ~WithLocalDeviceScale();
private:
  MAKE_UNMOVABLE(WithLocalDeviceScale);
  cairo_t *_cr;
  cairo_matrix_t _backup;
};

// Set the color of the cairo source
void setSourceColor(cairo_t *cr, const PlotUtils::HSV &hsv);
void setSourceColor(cairo_t *cr, const PlotUtils::RGB &rgb);

// Draws boat
void drawBoat(cairo_t *cr, double boatLength);

// Rotates counter-clockwise, in the "positive" direction
void rotateMathematically(cairo_t *cr, Angle<double> angle);

// Rotates clockwise, like like angles are applied when
// giving the heading of a boat on the map.
void rotateGeographically(cairo_t *cr, Angle<double> angle);

// Stroke with the visually same linewidth as
// in device coordinates
void deviceStroke(cairo_t *cr);

// Draw a single arrow
bool drawArrow(cairo_t *cr,
    const Eigen::Vector2d &from,
    const Eigen::Vector2d &to,
    double pointSize);

// Draw a couple of randomly translated arrows to visualize a flow
bool drawLocalFlow(
    cairo_t *cr,
    Eigen::Vector2d flowVector,
    double localStddev, int n,
    double pointSize,
    std::default_random_engine *rng);

void drawCross(cairo_t *cr, double size);

}
}

#endif /* SERVER_PLOT_CAIROUTILS_H_ */
