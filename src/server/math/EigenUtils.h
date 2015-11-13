/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_EIGENUTILS_H_
#define SERVER_MATH_EIGENUTILS_H_

#include <Eigen/Core>
#include <Eigen/Householder>
#include <Eigen/QR>

namespace sail {
namespace EigenUtils {

template <typename MatrixType>
MatrixType orthonormalBasis(MatrixType X) {
  Eigen::HouseholderQR<Eigen::MatrixXd> qr(X);
  auto selectSpanningSpace = Eigen::MatrixXd::Identity(X.rows(), X.cols());
  return qr.householderQ()*selectSpanningSpace;
}

struct ABPair {
  Eigen::MatrixXd A;
  Eigen::VectorXd B;
};

ABPair compress(const ABPair &pair);

}
}

#endif /* SERVER_MATH_EIGENUTILS_H_ */
