/*
 * CairoUtils.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#include <server/plot/CairoUtils.h>
#include <Eigen/Dense>

namespace sail {



std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x) {
  return std::shared_ptr<cairo_surface_t>(x, &cairo_surface_destroy);
}

std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x) {
  return std::shared_ptr<cairo_t>(x, &cairo_destroy);
}

WithLocalCairoCoordinates::WithLocalCairoCoordinates(cairo_t *cr) :
  _cr(cr) {
  cairo_get_matrix(_cr, &_backup);
}

WithLocalCairoCoordinates::~WithLocalCairoCoordinates() {
  cairo_set_matrix(_cr, &_backup);
}

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

template cairo_matrix_t toCairo<2, 3>(const Eigen::Matrix<double, 2, 3> &mat);
template cairo_matrix_t toCairo<2, 4>(const Eigen::Matrix<double, 2, 4> &mat);


} /* namespace sail */
