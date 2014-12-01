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

/*
 * TODO: Adjust the unit depending on the magnitude,
 *  e.g. display the distance in nautical miles if it is longer than
 *  1852 meters, or something like that.
 */

template <typename T>
std::ostream &operator<<(std::ostream &s, Duration<T> x) {
  s << x.seconds() << " seconds";
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, Length<T> x) {
  s << x.meters() << " meters";
  return s;
}

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
