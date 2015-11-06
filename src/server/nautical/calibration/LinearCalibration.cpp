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

namespace sail {
namespace LinearCalibration {

void initializeParameters(bool withOffset, double *dst) {
  int n = flowParamCount(withOffset);
  for (int i = 0; i < n; i++) {
    dst[i] = 0.0;
  }
  dst[0] = 1.0;
}

int calcPassiveCount(Duration<double> total, Duration<double> period) {
  return 1 + int(floor(total/period));
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

Array<Arrayi> makeRandomSplit(int sampleCount0, int splitCount) {
  int samplesPerSplit = sampleCount0/splitCount;
  int n = splitCount*samplesPerSplit;
  Arrayi inds(n);
  LineKM map(0, n, 0, splitCount);
  for (int i = 0; i < n; i++) {
    inds[i] = int(floor(map(i)));
  }
  std::random_shuffle(inds.begin(), inds.end());
  Array<ArrayBuilder<int> > splits(splitCount);
  for (int i = 0; i < n; i++) {
    splits[inds[i]].add(i);
  }
  return splits.map<Arrayi>([=](ArrayBuilder<int> b) {
    return b.get();
  });
}

Eigen::MatrixXd subtractMean(Eigen::MatrixXd A, int dim) {
  int rows = A.rows();
  int cols = A.cols();
  Eigen::MatrixXd dst = Eigen::MatrixXd(rows, cols);
  assert(rows % dim == 0);

  int count = rows/dim;
  assert(dim*count == rows);
  Eigen::MatrixXd sum = Eigen::MatrixXd::Zero(dim, cols);
  {
    int offset = 0;
    for (int i = 0; i < count; i++) {
      sum += A.block(offset, 0, dim, cols);
      offset += dim;
    }
  }
  sum *= (1.0/count);
  {
    int offset = 0;
    for (int i = 0; i < count; i++) {
      dst.block(offset, 0, dim, cols) = A.block(offset, 0, dim, cols) - sum;
      offset += dim;
    }
  }
  return dst;
}

Eigen::MatrixXd integrate(Eigen::MatrixXd A, int dim) {
  int rows = A.rows();
  int cols = A.cols();
  int count = rows/dim;
  Eigen::MatrixXd sum = Eigen::MatrixXd::Zero(dim, cols);
  Eigen::MatrixXd B((1 + count)*dim, cols);
  B.block(0, 0, dim, cols) = sum;
  for (int i = 0; i < count; i++) {
    int srcOffset = i*dim;
    int dstOffset = srcOffset + dim;
    auto a = A.block(srcOffset, 0, dim, cols);
    std::cout << "GOOD" << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(i) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(a) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(sum) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(B) << std::endl;
    sum += a;
    B.block(dstOffset, 0, dim, cols) = sum;
    std::cout << EXPR_AND_VAL_AS_STRING(B) << std::endl;
  }
  return B;
}

}
}
