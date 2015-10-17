/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/calibration/LinearCalibration.h>
#include <armadillo>
#include <server/math/BandMat.h>
#include <server/common/ArrayIO.h>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>


namespace sail {
namespace LinearCalibration {

void initializeLinearParameters(bool withOffset, double *dst2or4) {
  dst2or4[0] = 1.0;
  int n = (withOffset? 4 : 2);
  for (int i = 1; i < n; i++) {
    dst2or4[i] = 0.0;
  }
}

typedef Eigen::Triplet<double> Triplet;




int calcPassiveCount(Duration<double> total, Duration<double> period) {
  return 1 + int(floor(total/period));
}

CommonResults calibrateSparse(FlowMatrices rawMats, Duration<double> totalDuration,
    CommonCalibrationSettings settings) {
    return CommonResults();
}

LinearCorrector::LinearCorrector(const FlowSettings &flowSettings,
    Arrayd windParams, Arrayd currentParams) :
    _flowSettings(flowSettings), _windParams(windParams), _currentParams(currentParams) {}

Array<CalibratedNav<double> > LinearCorrector::operator()(const Array<Nav> &navs) const {
  return navs.map<CalibratedNav<double> >([&](const Nav &x) {
    return (*this)(x);
  });
}

arma::mat asMatrix(const MDArray2d &x) {
  return arma::mat(x.ptr(), x.rows(), x.cols(), false, true);
}

arma::mat asMatrix(const Arrayd &x) {
  return arma::mat(x.ptr(), x.size(), 1, false, true);
}

CalibratedNav<double> LinearCorrector::operator()(const Nav &nav) const {
  MDArray2d Aw(2, _flowSettings.windParamCount()), Bw(2, 1);
  MDArray2d Ac(2, _flowSettings.currentParamCount()), Bc(2, 1);

  makeTrueWindMatrixExpression(nav, _flowSettings, &Aw, &Bw);
  makeTrueCurrentMatrixExpression(nav, _flowSettings, &Ac, &Bc);
  arma::vec2 windMat = asMatrix(Aw)*asMatrix(_windParams) + asMatrix(Bw);
  arma::vec2 currentMat = asMatrix(Ac)*asMatrix(_currentParams) + asMatrix(Bc);

  HorizontalMotion<double> wind{Velocity<double>::knots(windMat[0]),
    Velocity<double>::knots(windMat[1])};
  HorizontalMotion<double> current{Velocity<double>::knots(currentMat[0]),
    Velocity<double>::knots(currentMat[1])};
  CalibratedNav<double> dst;
  dst.rawAwa.set(nav.awa());
  dst.rawAws.set(nav.aws());
  dst.rawMagHdg.set(nav.magHdg());
  dst.rawWatSpeed.set(nav.watSpeed());
  dst.gpsMotion.set(nav.gpsMotion());

  // TODO: Fill in more things here.

  dst.trueWindOverGround.set(wind);
  dst.trueCurrentOverGround.set(current);
  return dst;
}

std::string LinearCorrector::toString() const {
  std::stringstream ss;
  ss << "LinearCorrector(windParams="<< _windParams << ", currentParams=" << _currentParams << ")";
  return ss.str();
}


Results calibrate(CommonCalibrationSettings commonSettings,
    FlowSettings flowSettings, Array<Nav> navs) {
    assert(std::is_sorted(navs.begin(), navs.end()));
  auto totalDuration = navs.last().time() - navs.first().time();
  auto windResults = calibrateSparse(makeTrueWindMatrices(navs, flowSettings),
      totalDuration, commonSettings);
  auto currentResults = calibrateSparse(makeTrueCurrentMatrices(navs, flowSettings),
      totalDuration, commonSettings);
  //LinearCorrector corrector(flowSettings, windResults.parameters, currentResults.parameters);
  //return Results{corrector, windResults.recoveredFlow, currentResults.recoveredFlow};
  return Results();
}



}
}
