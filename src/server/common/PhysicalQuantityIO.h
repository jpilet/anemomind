/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PHYSICALQUANTITYIO_H_
#define PHYSICALQUANTITYIO_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/string.h>
#include <iosfwd>

namespace sail {

template <typename T>
std::ostream &operator<<(std::ostream &s, Velocity<T> x) {
  s << x.knots() << " knots";
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, Angle<T> x) {
  s << x.degrees() << " degrees";
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, HorizontalMotion<T> x) {
  s << "(x=" << x[0] << ", y=" << x[1] << ", dir=" << x.angle() << ", norm=" << x.norm() << ")";
  return s;
}

}

#endif /* PHYSICALQUANTITYIO_H_ */
