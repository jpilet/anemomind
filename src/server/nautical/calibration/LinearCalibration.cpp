/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/calibration/LinearCalibration.h>
#include <armadillo>



namespace sail {
namespace LinearCalibration {

HorizontalMotion<double> calcRawBoatMotion(
    Angle<double> magHdg, Velocity<double> watSpeed) {
    return HorizontalMotion<double>::polar(watSpeed, magHdg);
}

HorizontalMotion<double> calcRawTrueWind(
    Angle<double> magHdg,
    Angle<double> awa, Velocity<double> aws) {

  /*
   * awa is the angle relative to the boat.
   * awa + magHdg is the angle of the wind w.r.t. the geographic coordinate system.
   * awa + magHdg + 180degs is the angle in which the wind is blowing, not where it comes from
   */
  Angle<double> absoluteDirectionOfWind = magHdg + awa + Angle<double>::degrees(180.0);
  return HorizontalMotion<double>::polar(aws, absoluteDirectionOfWind);
}

void initializeLinearParameters(bool withOffset, double *dst2or4) {
  dst2or4[0] = 1.0;
  int n = (withOffset? 4 : 2);
  for (int i = 1; i < n; i++) {
    dst2or4[i] = 0.0;
  }
}

void armaTest() {
}

}
}
