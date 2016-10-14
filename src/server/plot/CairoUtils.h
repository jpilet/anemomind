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

namespace sail {

// Note on line-width, interesting!
// https://www.cairographics.org/tutorial/#L2linewidth

std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x);
std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x);

template <int rows, int cols>
cairo_matrix_t toCairo(
    const Eigen::Matrix<double, rows, cols> &mat);

class WithLocalCairoCoordinates {
public:
  WithLocalCairoCoordinates(cairo_t *cr);
  ~WithLocalCairoCoordinates();
private:
  WithLocalCairoCoordinates(
      const WithLocalCairoCoordinates &other) = delete;
  WithLocalCairoCoordinates &operator=(
      const WithLocalCairoCoordinates &other) = delete;
  cairo_t *_cr;
  cairo_matrix_t _backup;
};

class WithLocalCairoContext {
public:
  WithLocalCairoContext(cairo_t *cr);
  ~WithLocalCairoContext();
private:
  WithLocalCairoContext(const WithLocalCairoContext &x) = delete;
  WithLocalCairoContext &operator=(const WithLocalCairoContext &x) = delete;
  cairo_t *_cr;
};

void setCairoColor(cairo_t *cr, const PlotUtils::HSV &hsv);
void setCairoColor(cairo_t *cr, const PlotUtils::RGB &rgb);

void drawBoat(cairo_t *cr, double boatLength);

}

#endif /* SERVER_PLOT_CAIROUTILS_H_ */
