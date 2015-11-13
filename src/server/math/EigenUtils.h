/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_EIGENUTILS_H_
#define SERVER_MATH_EIGENUTILS_H_

#include <Eigen/Core>
#include <Eigen/Householder>
#include <Eigen/QR>
#include <server/math/Random.h>

namespace sail {
namespace EigenUtils {

Eigen::MatrixXd makeRandomMatrix(int rows, int cols, RandomEngine *rng, double s = 1.0);
bool eq(const Eigen::MatrixXd &a, const Eigen::MatrixXd &b, double tol = 1.0e-6);

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
