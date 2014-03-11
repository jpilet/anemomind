/*
 *  Created on: 11 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "DriftModel.h"
#include <server/nautical/DataCalib.h>

namespace sail {


adouble SinusDriftAngle::calcCorrectionAngle(BoatData *data, const Nav &nav, adouble *Xin) {
  adouble awaRadians = data->calcAwaRadians(nav, Xin);
  return preliminaryCourseErrorDueToDrift(awaRadians);
}

adouble SinusDriftAngle::preliminaryCourseErrorDueToDrift(adouble awaRadians) {
  const double maxAngle = deg2rad(5.0);
  adouble cosa = cos(awaRadians);
  if (cosa < 0) {
    return 0;
  } else {
    int sign = (sin(awaRadians) > 0? -1 : 1); // <-- is this correct?
    return sign*cosa;
  }
}

} /* namespace mmm */
