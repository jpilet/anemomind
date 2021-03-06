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

struct Setup {
  std::shared_ptr<cairo_surface_t> surface;
  std::shared_ptr<cairo_t> cr;
  static Setup svg(const std::string &filename,
        double width, double height);
};

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
// before, but the local scale will be one.
//
// This is useful for drawing things such as small marker, that should be
// invariant to most transformations except translation. Or maybe we want
// the boat drawing to be invariant to zoom.
class WithLocalDeviceScale {
public:
  enum Mode {Determinant, Identity, SVD};

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

void drawFilledCircle(cairo_t *cr, double r);

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

void renderPlot(
    // Settings for how the data should be renderered.
    const PlotUtils::Settings2d &settings,

    // Renders the data to be plotted
    std::function<void(cairo_t*)> dataRenderer,

    // Renders axes, labels, etc,
    // given the bounding box of the data
    std::function<void(BBox3d, cairo_t*)> contextRenderer,

    cairo_t *dst);

void renderPlot(
    const PlotUtils::Settings2d &settings,
    std::function<void(cairo_t*)> dataRenderer,
    const std::string &xLabel,
    const std::string &yLabel,
    cairo_t *dst);

void moveTo(cairo_t *dst, const Eigen::Vector2d &x);
void lineTo(cairo_t *dst, const Eigen::Vector2d &x);
void plotLineStrip(cairo_t *dst,
    const Array<Eigen::Vector2d> &src);
void plotDots(cairo_t *dst,
    const Array<Eigen::Vector2d> &pts,
    double dotSize);

}
}

#endif /* SERVER_PLOT_CAIROUTILS_H_ */
