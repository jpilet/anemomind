/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_

namespace sail {
namespace LinearCalibration {

HorizontalMotion<double> calcRawBoatMotion(
    Angle<double> magHdg, Velocity<double> watSpeed);

HorizontalMotion<double> calcRawApparentWind(
    Angle<double> magHdg,
    Angle<double> awa, Velocity<double> aws);

// Given a motion, the calibrated motion
// is the matrix product of the output matrix
// of this function multiplied by a vector of two
// calibration parameters [a; b]. The calibration consists
// of a rotation and scaling. When there is no calibration,
// then a=1 and b=0.
template <typename MatrixType>
void makeMotionCalibrationMatrix(double motion[2], MatrixType *dst) {
  (*dst)(0, 0) = motion[0];
  (*dst)(0, 1) = -motion[1];
  (*dst)(1, 0) = motion[1];
  (*dst)(1, 1) = motion[0];
}


// Express the local current as a function
//
// Current = A*[a; b] + B
//
// where A is a 2x2 matrix, [a; b] are calibration parameters,
// and B is a 2x1 matrix.
//
// The norm of [a; b] is a correction of the scaling error of the water speed sensor.
// The angle (atan2(b, a)) is the correction applied to magnetic heading.
template <typename MatrixType>
void makeLinearCurrentExpr(
    HorizontalMotion<double> gpsMotion,
    Angle<double> magHdg, Velocity<double> watSpeed,
    MatrixType *Adst, MatrixType *Bdst,
    Velocity<double> unit = Velocity<double>::knots(1.0)) {
  HorizontalMotion<double> rawBoatMotion = calcRawBoatMotion(magHdg, watSpeed);
  double motion[2] = {-rawBoatMotion[0]/unit, -rawBoatMotion[1]/unit};
  makeMotionCalibrationMatrix(motion, Adst);
  (*Bdst)(0, 0) = gpsMotion[0]/unit;
  (*Bdst)(1, 0) = gpsMotion[1]/unit;
}

// Make a linear matrix expression to compute the
// true wind as a vector of two parameters [a; b],
// just like for the current. In this case,
// both the error of the magnetic compass and the wind angle sensor
// is corrected for at the same time.
template <typename MatrixType>
void makeLinearWindExpr(
    HorizontalMotion<double> gpsMotion,
    Angle<double> rawMagHdg,
    Angle<double> rawAwa, Velocity<double> rawAws,
    MatrixType *Adst, MatrixType *Bdst,
  Velocity<double> unit = Velocity<double>::knots(1.0)) {
  HorizontalMotion<double> rawApparentWind =
      calcRawApparentWind(rawMagHdg, rawAwa, rawAws);
  double motion[2] = {rawApparentWind[0]/unit, rawApparentWind[1]/unit};
  makeMotionCalibrationMatrix(motion, Adst);
  (*Bdst)(0, 0) = gpsMotion[0]/unit;
  (*Bdst)(1, 0) = gpsMotion[1]/unit;
}

}
}

#endif
