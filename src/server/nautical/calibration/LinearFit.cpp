/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/calibration/LinearFit.h>
#include <cassert>

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
  auto Xcoefsh = Xflow.solve();
  auto Ycoefsh = Yflow.solve();
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


}
}
