/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/calibration/LinearFit.h>
#include <server/math/Integral1d.h>
#include <cassert>
#include <server/common/Functional.h>

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


}
}
