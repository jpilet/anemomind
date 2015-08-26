/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {
namespace LinearCalibration {

/*
 * See docs/calib/linearcalib.tex
 *
 * If withOffset is false, dst is assumed to be a 2x2 matrix,
 * otherwise a 2x4 matrix. The dst matrix, when multiplied with
 * a parameter vector, results in a calibrated motion.
 */
template <typename MatrixType>
void makeCalibratedMotionMatrix(
    Angle<double> angle, Velocity<double> magnitude,
    bool withOffset,
    MatrixType *dst, Velocity<double> unit) {
  double cosPhi = cos(angle);
  double sinPhi = sin(angle);
  double r = magnitude/unit;
  (*dst)(0, 0) = r*sinPhi; (*dst)(0, 1) = -r*cosPhi;
  (*dst)(1, 0) = r*cosPhi; (*dst)(1, 1) = r*sinPhi;
  if (withOffset) {
    (*dst)(0, 2) = sinPhi; (*dst)(0, 3) = -cosPhi;
    (*dst)(1, 2) = cosPhi; (*dst)(1, 3) = sinPhi;
  }
}

template <typename InstrumentAbstraction>
HorizontalMotion<double> getGpsMotion(const InstrumentAbstraction &nav) {
  return HorizontalMotion<double>::polar(nav.gpsSpeed(), nav.gpsBearing());
}

template <typename MatrixType>
void makeGpsOffset(const HorizontalMotion<double> &m, MatrixType *dstB,
    Velocity<double> unit) {
  (*dstB)(0, 0) = m[0]/unit;
  (*dstB)(1, 0) = m[1]/unit;
}


/*
 * Use this function to express the true wind W as a function of the parameters X:
 *
 * W(X) = AX + B
 *
 * If withOffset = false, then A is 2x2, otherwise if it is true, then A is 2x4
 * B is always 2x1
 */
template <typename InstrumentAbstraction, typename MatrixType>
void makeTrueWindMatrixExpression(const InstrumentAbstraction &nav,
  bool withOffset,
  MatrixType *dstA, MatrixType *dstB,
  Velocity<double> unit = Velocity<double>::knots(1.0)) {
  auto absoluteDirectionOfWind = nav.magHdg() + nav.awa() + Angle<double>::degrees(180);
  makeCalibratedMotionMatrix(absoluteDirectionOfWind, nav.aws(), withOffset, dstA, unit);
  makeGpsOffset(getGpsMotion(nav), dstB, unit);
}

/*
 * Use this function to express the true current C as a function of the parameters X:
 *
 * C(X) = AX + B
 *
 * If withOffset = false, then A is 2x2, otherwise if it is true, then A is 2x4
 * B is always 2x1
 */
template <typename InstrumentAbstraction, typename MatrixType>
void makeTrueCurrentMatrixExpression(const InstrumentAbstraction &nav,
  bool withOffset,
  MatrixType *dstA, MatrixType *dstB,
  Velocity<double> unit = Velocity<double>::knots(1.0)) {
  auto oppositeDirectionOfBoatOverWater = nav.magHdg() + Angle<double>::degrees(180);
  makeCalibratedMotionMatrix(oppositeDirectionOfBoatOverWater,
      nav.watSpeed(), withOffset, dstA, unit);
  makeGpsOffset(getGpsMotion(nav), dstB, unit);
}

void initializeLinearParameters(bool withOffset, double *dst2or4);

}
}

#endif
