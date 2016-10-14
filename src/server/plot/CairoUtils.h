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

// Note on line-width, interesting!
// https://www.cairographics.org/tutorial/#L2linewidth
// https://www.cairographics.org/manual/cairo-Transformations.html

std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x);
std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x);

template <int rows, int cols>
cairo_matrix_t toCairo(
    const Eigen::Matrix<double, rows, cols> &mat);

class WithLocalCairoContext {
public:
  WithLocalCairoContext(cairo_t *cr);
  ~WithLocalCairoContext();
private:
  MAKE_UNMOVABLE(WithLocalCairoContext);
  cairo_t *_cr;
};

// This hack only works as expected
// if the transformation is orthogonal
class WithDeviceLineWidth {
public:
  WithDeviceLineWidth(cairo_t *cr);
  ~WithDeviceLineWidth();
private:
  MAKE_UNMOVABLE(WithDeviceLineWidth);
  cairo_t *_cr;
  double _lw;
};

void setCairoSourceColor(cairo_t *cr, const PlotUtils::HSV &hsv);
void setCairoSourceColor(cairo_t *cr, const PlotUtils::RGB &rgb);

void drawBoat(cairo_t *cr, double boatLength);

void rotateMathematically(cairo_t *cr, Angle<double> angle);
void rotateGeographically(cairo_t *cr, Angle<double> angle);

bool drawArrow(cairo_t *cr,
    const Eigen::Vector2d &from,
    const Eigen::Vector2d &to,
    double pointSize);

bool drawLocalFlow(
    cairo_t *cr,
    Eigen::Vector2d flowVector,
    double localStddev, int n,
    double pointSize,
    std::default_random_engine *rng);

}

#endif /* SERVER_PLOT_CAIROUTILS_H_ */
