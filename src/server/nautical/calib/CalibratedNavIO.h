/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATEDNAVIO_H_
#define CALIBRATEDNAVIO_H_

#include <server/common/PhysicalQuantityIO.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>

namespace sail {

template <typename T>
std::ostream &operator<<(std::ostream &s, CalibratedNav<T> n) {
  s << EXPR_AND_VAL_AS_STRING(n.gpsMotion()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawAwa()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawMagHdg()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawAws()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawWatSpeed()) << std::endl;

  // Values that need to be calibrated externally.
  s << EXPR_AND_VAL_AS_STRING(n. calibAwa()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.boatOrientation()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.calibAws()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.calibWatSpeed()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n. driftAngle()) << std::endl; // <-- Optional to calibrate.

  // Values that are populated using the fill(n.()) method.
  // Depend on the calibrated values.
  s << EXPR_AND_VAL_AS_STRING(n.apparentWindAngleWrtEarth()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.apparentWind()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.trueWind()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.trueCurrent()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.boatMotionThroughWater()) << std::endl;
  return s;
}

}



#endif /* CALIBRATEDNAVIO_H_ */
