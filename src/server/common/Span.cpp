/*
 * Span.cpp
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#include "Span.h"
#include <algorithm>
#include <iostream>

namespace sail {


std::ostream &operator<<(std::ostream &s, const Spand &x) {
  s << "Span(" << x.minv() << ", " << x.maxv() << ")";
  return s;
}


} /* namespace sail */
