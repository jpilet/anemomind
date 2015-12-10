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

}
}
