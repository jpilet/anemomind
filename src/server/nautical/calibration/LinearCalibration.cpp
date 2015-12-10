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
#include <server/math/nonlinear/DataFit.h>
#include <server/plot/extra.h>
#include <server/common/ArrayBuilder.h>
#include <ceres/ceres.h>
#include <server/math/EigenUtils.h>
#include <server/common/Functional.h>

namespace sail {
namespace LinearCalibration {

void initializeParameters(bool withOffset, double *dst) {
  int n = flowParamCount(withOffset);
  for (int i = 0; i < n; i++) {
    dst[i] = 0.0;
  }
  dst[0] = 1.0;
}

Arrayd makeXinit(bool withOffset) {
  int n = flowParamCount(withOffset);
  Arrayd dst(n);
  initializeParameters(withOffset, dst.ptr());
  return dst;
}

Eigen::VectorXd makeXinitEigen() {
  Eigen::VectorXd dst = Eigen::VectorXd::Zero(4);
  initializeParameters(true, dst.data());
  return dst;
}

int calcPassiveCount(Duration<double> total, Duration<double> period) {
  return 1 + int(floor(total/period));
}

LinearCorrector::LinearCorrector(const FlowSettings &flowSettings,
    Arrayd windParams, Arrayd currentParams) :
    _flowSettings(flowSettings), _windParams(windParams), _currentParams(currentParams) {}

Array<CalibratedNav<double> > LinearCorrector::operator()(const Array<Nav> &navs) const {
  return map(navs, [&](const Nav &x) {
    return (*this)(x);
  }).toArray();
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

Array<Arrayi> makeRandomSplit(int sampleCount0, int splitCount, RandomEngine *rng) {
  int samplesPerSplit = sampleCount0/splitCount;
  int n = splitCount*samplesPerSplit;
  Arrayi inds(n);
  LineKM m(0, n, 0, splitCount);
  for (int i = 0; i < n; i++) {
    inds[i] = int(floor(m(i)));
  }

  if (rng == nullptr) {
    std::random_shuffle(inds.begin(), inds.end());
  } else {
    auto gen = [&](int index) {
      return std::uniform_int_distribution<int>(0, index)(*rng);
    };
    std::random_shuffle(inds.begin(), inds.end(), gen);
  }

  Array<ArrayBuilder<int> > splits(splitCount);
  for (int i = 0; i < n; i++) {
    splits[inds[i]].add(i);
  }
  return map(splits, [=](ArrayBuilder<int> b) {
    return b.get();
  }).toArray();
}

Array<Spani> makeContiguousSpans(int sampleCount, int splitSize) {
  int splitCount = sampleCount/splitSize;
  return map(Spani(0, splitCount), [&](int index) {
    int from = index*splitSize;
    int to = from + splitSize;
    return Spani(from, to);
  }).toArray();
}


Array<Spani> makeOverlappingSpans(int sampleCount, int splitSize, double relStep) {
  double step = relStep*splitSize;
  int lastIndex = int(floor((sampleCount - splitSize)/step));
  int count = lastIndex+1;
  LineKM from(0, count-1, 0.0, sampleCount - splitSize);
  LineKM to(0, count-1, splitSize, sampleCount);
  return map(Spani(0, count), [=](int i) {
    return Spani(int(round(from(i))), int(round(to(i))));
  }).toArray();
}









/*Array<NormedData> assembleNormedData(FlowMatrices mats, Array<Arrayi> splits) {

}*/



}
}
