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

#define MAKE_UNMOVABLE(ClassName) \
  ClassName(const ClassName &) = delete; \
  ClassName &operator=(const ClassName &) = delete;

namespace sail {
namespace Cairo {

// Note on line-width, interesting!
// https://www.cairographics.org/tutorial/#L2linewidth
// https://www.cairographics.org/manual/cairo-Transformations.html

std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x);
std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x);

template <int rows, int cols>
cairo_matrix_t toCairo(
    const Eigen::Matrix<double, rows, cols> &mat);

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
// before, but the local scale will be different.
//
//
// This is useful for drawing things such as small marker, that should be
// invariant to most transformations except translation.
class WithLocalDeviceScale {
public:
  enum Mode {Determinant, Identity};

  WithLocalDeviceScale(cairo_t *cr, Mode mode = Identity);
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

// Rotates counter-clockwise
void rotateMathematically(cairo_t *cr, Angle<double> angle);

// Rotates clockwise
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

}
}

#endif /* SERVER_PLOT_CAIROUTILS_H_ */
