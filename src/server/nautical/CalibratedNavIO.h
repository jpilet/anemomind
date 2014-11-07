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
  s << EXPR_AND_VAL_AS_STRING(n.gpsMotion.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawAwa.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawMagHdg.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawAws.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.rawWatSpeed.get()) << std::endl;

  // Values that need to be calibrated externally.
  s << EXPR_AND_VAL_AS_STRING(n. calibAwa.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.boatOrientation.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.calibAws.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.calibWatSpeed.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n. driftAngle.get()) << std::endl; // <-- Optional to calibrate.

  // Values that are populated using the fill(n..get()) method.
  // Depend on the calibrated values.
  s << EXPR_AND_VAL_AS_STRING(n.apparentWindAngleWrtEarth.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.apparentWind.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.trueWind.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.trueCurrent.get()) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(n.boatMotionThroughWater.get()) << std::endl;
  return s;
}

}



#endif /* CALIBRATEDNAVIO_H_ */
