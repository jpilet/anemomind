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

  const double k = 1.5395;

  adouble cosa = cos(awaRadians);
  if (cosa.getValue() > 0) {
    adouble w = (sin(3*awaRadians) + sin(awaRadians));
    return (maxAngle/k)*w;
  } else {
    return 0.0;
  }
}

} /* namespace mmm */
