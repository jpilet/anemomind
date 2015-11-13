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

bool isOrthonormal(Eigen::MatrixXd Q, double tol = 1.0e-9);
Eigen::MatrixXd projector(Eigen::MatrixXd A);
bool spanTheSameSubspace(Eigen::MatrixXd A, Eigen::MatrixXd B, double tol = 1.0e-9);

Eigen::MatrixXd makeRandomMatrix(int rows, int cols, RandomEngine *rng, double s = 1.0);
bool eq(const Eigen::MatrixXd &a, const Eigen::MatrixXd &b, double tol = 1.0e-6);

template <typename MatrixType>
MatrixType orthonormalBasis(MatrixType X) {
  Eigen::HouseholderQR<Eigen::MatrixXd> qr(X);
  auto selectSpanningSpace = Eigen::MatrixXd::Identity(X.rows(), X.cols());
  return qr.householderQ()*selectSpanningSpace;
}

template <typename MatrixType>
MatrixType nullspace(MatrixType X) {
  Eigen::HouseholderQR<Eigen::MatrixXd> qr(X);
  auto selectNullspace = Eigen::MatrixXd::Identity(X.rows(), X.rows())
    .block(0, X.cols(), X.rows(), X.rows() - X.cols());
  return qr.householderQ()*selectNullspace;
}

struct ABPair {
  Eigen::MatrixXd A;
  Eigen::VectorXd B;
};

ABPair compress(const ABPair &pair);


// Minimize w.r.t. X: |A*X|^2/|B*X|^2
// The scale of the solution is undefined.
// This function just returns one solution that
// can be scaled. The columns of B must be linearly independent,
// and B should generally have more rows than columns.
Eigen::VectorXd minimizeNormRatio(Eigen::MatrixXd A,
                                  Eigen::MatrixXd B);

// Minimize w.r.t. X: |A*X + B|^2/|C*X + D|^2.
// The columns of C and D should all be linearly independent
// from each other.
Eigen::VectorXd minimizeNormRatio(Eigen::MatrixXd A, Eigen::VectorXd B,
                                  Eigen::MatrixXd C, Eigen::VectorXd D);


}
}

#endif /* SERVER_MATH_EIGENUTILS_H_ */
