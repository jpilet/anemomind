/*
 *  Created on: 2014-09-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Polar coordinates.
 */

#ifndef POLARCOORDINATES_H_
#define POLARCOORDINATES_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {
  // Functions to map polar coordinates to cartesian ones.
  // 'nautical = true' means that the coordinates are mapped so that
  //   an angle of 0 points upward in an x-y-plot and when it increases,
  //   it goes clockwise in the plane, like the compass directions on a map.
  // 'nautical = false' means the the coordinates are mapped to the plane the way
  //   mathematicians are used to, with an angle of 0 pointing to the right and
  //   increasing angles mean a counter-clockwise movement in the plane.
  template <typename RadiusType, typename AngleType>
  RadiusType calcPolarX(bool nautical, RadiusType radius, Angle<AngleType> angle) {
    return radius*(nautical? sin(angle) : cos(angle));
  }

  template <typename RadiusType, typename AngleType>
  RadiusType calcPolarY(bool nautical, RadiusType radius, Angle<AngleType> angle) {
    return calcPolarX(!nautical, radius, angle);
  }
}



#endif /* POLARCOORDINATES_H_ */
