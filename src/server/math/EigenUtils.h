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
#include <server/common/Span.h>
#include <server/common/MDArray.h>

namespace sail {
namespace EigenUtils {

template <typename T>
Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > arrayToEigen(Array<T> src) {
  return Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> >(src.ptr(), src.size(), 1);
}

template <typename T>
Eigen::Map<T> arrayToEigen(MDArray<T, 2> src) {
  return Eigen::Map<T>(src.ptr(), src.rows(), src.cols());
}

template <typename VectorType>
auto vectorToArray(VectorType x) -> Array<decltype(x.norm())> {
  typedef decltype(x.norm()) T;
  int size = x.size();
  T *data = x.data();
  return Array<T>(size, data);
}

template <typename MatrixType>
auto sliceRows(MatrixType &A, int from, int to) -> decltype(A.block(0, 0, 1, 1)) {
  return A.block(from, 0, to-from, A.cols());
}

template <typename MatrixType>
auto sliceCols(MatrixType &A, int from, int to) -> decltype(A.block(0, 0, 1, 1)) {
  return A.block(0, from, A.rows(), to-from);
}

template <typename MatrixType>
auto sliceRows(MatrixType &A, Spani s) -> decltype(sliceRows<MatrixType>(A, 0, 1)) {
  return sliceRows(A, s.minv(), s.maxv());
}

template <typename MatrixType>
auto sliceCols(MatrixType &A, Spani s) -> decltype(sliceCols<MatrixType>(A, 0, 1)) {
  return sliceCols(A, s.minv(), s.maxv());
}

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
