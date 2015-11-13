/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include "EigenUtils.h"

namespace sail {
namespace EigenUtils {

ABPair compress(const ABPair &pair) {
  auto Q = orthonormalBasis(pair.A);
  Eigen::MatrixXd R = Q.transpose()*pair.A;
  Eigen::VectorXd QtB = Q.transpose()*pair.B;
  return ABPair{R, QtB};
}

}
} /* namespace sail */
