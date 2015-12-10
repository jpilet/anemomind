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
Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > arrayToEigen(MDArray<T, 2> src) {
  return Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> >(
      src.ptr(), src.rows(), src.cols()
    );
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

// Since matrices often appear as
// pairs, e.g. in least-squares problems,
// it makes sense to have a struct for that.
struct MatrixPair {
  MatrixPair(const Eigen::MatrixXd &a, const Eigen::MatrixXd &b) : A(a), B(b) {}

  // Constructor to make this data type compatible with templated algorithms,
  // such as integral images, where we would usually add up scalars, initializing
  // the integral sum with 0.
  MatrixPair(double x) {assert(x == 0.0);}

  Eigen::MatrixXd A;
  Eigen::MatrixXd B;

  bool empty() const;
  MatrixPair operator+(const MatrixPair &other) const;
  MatrixPair operator*(double factor) const;
};

// Returns a pair of matrices (a, b), so that
// a'*a = A'*A and b'*b = B'*B
// a and b have exactly A.cols() rows. This is useful in order
// to rewrite a least-squares problem to a smaller, but equivalent one,
// because the normal equations of the problem Minimize_X ||A*X - B||^2 are the same
// as for the problem Minimize_X ||a*X - B||^2
MatrixPair compress(const Eigen::MatrixXd &A, const Eigen::MatrixXd &B);

// Returns (A'A, A'B). Useful when building least-squares problems incrementally, since
// if we let A = [A0; A1] and B = [B0; B1] and we
// want to minimize || A*X - B ||^2, then we can compute
// (A'A, A'B) = (A0'A0, A0'B0) + (A1'A1, A1'B1), that is,
// we don't need to compute the A and B matrices explicitly in order to build
// the normal equations. This can be combined with integral images in order
// to efficiently build the normal equations for an arbitrary row-slice of A and B.
MatrixPair makeNormalEqs(Eigen::MatrixXd A, Eigen::MatrixXd B);


}
}

#endif /* SERVER_MATH_EIGENUTILS_H_ */
