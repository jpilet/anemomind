/*
 * CairoUtils.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#include "CairoUtils.h"

namespace sail {



std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x) {
  return std::shared_ptr<cairo_surface_t>(x, &cairo_surface_destroy);
}

std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x) {
  return std::shared_ptr<cairo_t>(x, &cairo_destroy);
}

} /* namespace sail */
