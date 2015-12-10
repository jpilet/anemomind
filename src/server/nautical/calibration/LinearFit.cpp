/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/calibration/LinearFit.h>
#include <server/math/Integral1d.h>
#include <cassert>
#include <server/common/Functional.h>
#include <server/math/Majorize.h>

namespace sail {
namespace LinearFit {

EigenUtils::MatrixPair buildNormalEqs(
    Angle<double> heading,
    const Eigen::MatrixXd &A1, const Eigen::MatrixXd &B1) {
  return buildNormalEqs(heading, EigenUtils::hcat(A1, B1));
}

EigenUtils::MatrixPair buildNormalEqs(
    Angle<double> heading,
    const Eigen::MatrixXd &AB1) {
  assert(AB1.rows() == 1);
  Eigen::MatrixXd K(1, 3);
  K(0, 0) = cos(heading);
  K(0, 1) = sin(heading);
  K(0, 2) = 1.0;
  return EigenUtils::MatrixPair(K.transpose()*K, K.transpose()*AB1);
}

EigenUtils::MatrixPair makeXYCoefMatrices(EigenUtils::MatrixPair Xflow,
                                          EigenUtils::MatrixPair Yflow) {
  auto Xcoefsh = Xflow.luSolve();
  auto Ycoefsh = Yflow.luSolve();
  assert(Xcoefsh.rows() == 3);
  assert(Ycoefsh.rows() == 3);
  assert(Xcoefsh.cols() == Ycoefsh.cols());
  int cols = Xcoefsh.cols();
  int lastCol = cols-1;
  assert(0 <= lastCol);
  Eigen::MatrixXd A(4, lastCol);
  Eigen::MatrixXd B(4, 1);
  constexpr int x_ = 0;
  constexpr int y_ = 2;
  for (int i = 0; i < 2; i++) {
    B(i + x_, 0) = Xcoefsh(i, lastCol);
    B(i + y_, 0) = Ycoefsh(i, lastCol);
    for (int j = 0; j < lastCol; j++) {
      A(i + x_, j) = Xcoefsh(i, j);
      A(i + y_, j) = Ycoefsh(i, j);
    }
  }
  return EigenUtils::MatrixPair(A, B);
}

Array<EigenUtils::MatrixPair> makeNormalEqs(Array<Angle<double> > headings,
                                            EigenUtils::MatrixPair flowEqs,
                                            int dim) {
  using namespace EigenUtils;
  int n = headings.size();
  assert(2*n == flowEqs.rows());
  assert(dim == 0 || dim == 1);
  Array<MatrixPair> dst(n);
  for (int i = 0; i < n; i++) {
    int offset = 2*i;
    int from = offset + dim;
    int to = from + 1;
    dst[i] = buildNormalEqs(headings[i],
                            sliceRows(flowEqs.A, from, to),
                            sliceRows(flowEqs.B, from, to));
  }
  return dst;
}

Array<EigenUtils::MatrixPair> makeCoefMatrices(Array<EigenUtils::MatrixPair> X,
                                               Array<EigenUtils::MatrixPair> Y,
                                               Array<Spani> spans) {
  using namespace EigenUtils;
  Integral1d<MatrixPair> Xitg(X);
  Integral1d<MatrixPair> Yitg(Y);
  assert(X.size() == Y.size());
  int n = spans.size();
  Array<MatrixPair> dst(n);
  for (int i = 0; i < n; i++) {
    auto s = spans[i];
    dst[i] = makeXYCoefMatrices(Xitg.integrate(s), Yitg.integrate(s));
  }
  return dst;
}

namespace {
  EigenUtils::MatrixPair coefMatricesToNormalEqs(EigenUtils::MatrixPair p) {
    return EigenUtils::makeNormalEqs(p.A, -p.B);
  }
}

Eigen::VectorXd minimizeLeastSquares(Array<EigenUtils::MatrixPair> coefMatrices) {
  return sail::map(coefMatrices, coefMatricesToNormalEqs)
    .reduce([](const EigenUtils::MatrixPair &a,
               const EigenUtils::MatrixPair &b) {
    return a + b;
  }).luSolve();
}

namespace {
  EigenUtils::MatrixPair computeWeightedNormalEqs(const EigenUtils::MatrixPair &p,
                                                  const Eigen::VectorXd &X) {
    auto Y = p.A*X + p.B;
    double error = Y.norm();
    auto maj = MajQuad::majorizeAbs(error, 1.0e-9);
    return maj.a*coefMatricesToNormalEqs(p);
  }

  Eigen::VectorXd iterateLeastSumOfNorms(const Array<EigenUtils::MatrixPair> &coefMatrices,
                                         const Eigen::VectorXd &X) {
    EigenUtils::MatrixPair acc;
    for (auto cm: coefMatrices) {
      acc = computeWeightedNormalEqs(cm, X);
    }
    return acc.luSolve();
  }

}

Eigen::VectorXd minimizeLeastSumOfNorms(Array<EigenUtils::MatrixPair> coefMatrices) {
  Eigen::VectorXd X = minimizeLeastSquares(coefMatrices);
  int iters = 30;
  for (int i = 0; i < iters; i++) {
    X = iterateLeastSumOfNorms(coefMatrices, X);
  }
  return X;
}

Eigen::VectorXd minimizeCoefs(Array<EigenUtils::MatrixPair> coefMatrices, int exponent) {
  assert(exponent == 1 || exponent == 2);
  if (exponent == 1) {
    return minimizeLeastSumOfNorms(coefMatrices);
  }
  return minimizeLeastSquares(coefMatrices);
}

namespace {
  Array<HorizontalMotion<double> > calibrateFlow(Array<Angle<double> > hdg,
                     Array<Nav> navs,
                     Settings settings,
                     LinearCalibration::FlowSettings flowSettings, bool wind) {
    int n = navs.size();
    assert(n == hdg.size());
    auto mats = LinearCalibration::makeTrueFlowMatrices(navs, flowSettings, wind);
    assert(mats.rows() == 2*n);
    auto X = makeNormalEqs(hdg, mats, 0);
    auto Y = makeNormalEqs(hdg, mats, 1);
    auto spans = LinearCalibration::makeOverlappingSpans(
        navs.size(), settings.spanSize, settings.relativeStep);
    auto coefs = makeCoefMatrices(X, Y, spans);
    auto params = minimizeCoefs(coefs, settings.exponent);
    Eigen::VectorXd correctedFlow = mats.A*params + mats.B;
    Array<HorizontalMotion<double> > results(n);
    for (int i = 0; i < n; i++) {
      int offset = 2*i;
      double cx = correctedFlow(offset + 0);
      double cy = correctedFlow(offset + 1);
      results[i] = HorizontalMotion<double>(cx*LinearCalibration::unit(),
                                            cy*LinearCalibration::unit());
    }
    return results;
  }
}

Array<CalibratedNav<double> > LinearFitCorrectorFunction::operator()(const Array<Nav> &navs) const {
  auto hdg = getMagHdg(navs);
  int n = navs.size();
  auto wind = calibrateFlow(hdg, navs, _settings, _flowSettings, true);
  auto current = calibrateFlow(hdg, navs, _settings, _flowSettings, false);
  assert(wind.size() == n);
  assert(current.size() == n);
  Array<CalibratedNav<double> > dst(n);
  for (int i = 0; i < n; i++) {
    dst[i].trueWindOverGround.set(wind[i]);
    dst[i].trueCurrentOverGround.set(current[i]);
  }
  return dst;
}

}
}
