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

namespace {

template <typename T>
void output(std::ostream &s, const Span<T> &x) {
  s << "Span(" << x.minv() << ", " << x.maxv() << ")";
}

}

std::ostream &operator<<(std::ostream &s, const Span<int> &x) {
  output(s, x);
  return s;
}

std::ostream &operator<<(std::ostream &s, const Span<double> &x) {
  output(s, x);
  return s;
}



} /* namespace sail */
