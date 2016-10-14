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

namespace sail {

std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x);
std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x);

}

#endif /* SERVER_PLOT_CAIROUTILS_H_ */
