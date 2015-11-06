/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>
#include <server/math/irls.h>

namespace sail {
namespace LinearCalibration {

void initializeParameters(bool withOffset, double *dst);

inline int flowParamCount(bool withOffset) {
  return (withOffset? 4 : 2);
}

struct FlowSettings {
 bool windWithOffset = true;
 bool currentWithOffset = true;

 int windParamCount() const {
   return flowParamCount(windWithOffset);
 }

 int currentParamCount() const {
   return flowParamCount(currentWithOffset);
 }
};


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
  FlowSettings settings,
  MatrixType *dstA, MatrixType *dstB,
  Velocity<double> unit = Velocity<double>::knots(1.0)) {
  auto absoluteDirectionOfWind = nav.magHdg() + nav.awa() + Angle<double>::degrees(180);
  makeCalibratedMotionMatrix(absoluteDirectionOfWind, nav.aws(), settings.windWithOffset, dstA, unit);
  makeGpsOffset(getGpsMotion(nav), dstB, unit);
}


struct FlowMatrices {
 MDArray2d A, B;
};

template <typename InstrumentAbstraction>
FlowMatrices makeTrueWindMatrices(Array<InstrumentAbstraction> navs, const FlowSettings &s) {
  int n = navs.size();
  int paramCount = s.windParamCount();
  MDArray2d A(2*n, paramCount);
  MDArray2d B(2*n, 1);
  for (int i = 0; i < n; i++) {
    int offset = 2*i;
    auto a = A.sliceRowBlock(i, 2);
    auto b = B.sliceRowBlock(i, 2);
    makeTrueWindMatrixExpression(navs[i], s, &a, &b);
  }
  return FlowMatrices{A, B};
}

template <typename InstrumentAbstraction>
FlowMatrices makeTrueCurrentMatrices(Array<InstrumentAbstraction> navs, const FlowSettings &s) {
  int n = navs.size();
  int paramCount = s.currentParamCount();
  MDArray2d A(2*n, paramCount);
  MDArray2d B(2*n, 1);
  for (int i = 0; i < n; i++) {
    int offset = 2*i;
    auto a = A.sliceRowBlock(i, 2);
    auto b = B.sliceRowBlock(i, 2);
    makeTrueCurrentMatrixExpression(navs[i], s, &a, &b);
  }
  return FlowMatrices{A, B};
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
  const FlowSettings &s,
  MatrixType *dstA, MatrixType *dstB,
  Velocity<double> unit = Velocity<double>::knots(1.0)) {
  auto oppositeDirectionOfBoatOverWater = nav.magHdg() + Angle<double>::degrees(180);
  makeCalibratedMotionMatrix(oppositeDirectionOfBoatOverWater,
      nav.watSpeed(), s.currentWithOffset, dstA, unit);
  makeGpsOffset(getGpsMotion(nav), dstB, unit);
}


/*
 * Perform full calibration of wind and current.
 */
// A class used to map a raw nav to a corrected one, using the parameters recovered.
class LinearCorrector : public CorrectorFunction {
 public:
  LinearCorrector() {}
  LinearCorrector(const FlowSettings &flowSettings, Arrayd windParams, Arrayd currentParams);
  Array<CalibratedNav<double> > operator()(const Array<Nav> &navs) const;
  CalibratedNav<double> operator()(const Nav &navs) const;
  std::string toString() const;
 private:
  FlowSettings _flowSettings;
  Arrayd _windParams, _currentParams;
};


struct CalibrationSettings {
  irls::Settings irlsSettings;
  int samplesPerSpan = 60;
  double inlierFrac = 0.2;
};

struct PlotData {
  MDArray2d Xflow, Yflow, Xgps, Ygps;
};

struct Results {
  Arrayd parameters;
  Array<Spani> spans;
  Arrayi inliers;
  Arrayb inlierMask;
};


Results calibrate(FlowMatrices mats, const CalibrationSettings &s);

}
}

#endif
