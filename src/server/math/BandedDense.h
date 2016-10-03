/*
 * BandedDense.h
 *
 *  Created on: 3 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_BANDEDDENSE_H_
#define SERVER_MATH_BANDEDDENSE_H_

#include <Eigen/Dense>
#include <server/math/lapack/BandMatrix.h>
#include <server/common/logging.h>
#include <server/math/lapack/BandWrappers.h>

namespace sail {

template <typename T>
MDArray<T, 2> wrap(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &m) {
  return MDArray<T, 2>(m.rows(), m.cols(), m.data());
}


template <typename T>
void addToDiag(int left,
    const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &A0,
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> *m0Scratch,
    SymmetricBandMatrixL<T> *P) {
  int m0 = A0.cols();
  m0Scratch->block(0, 0, m0, m0) = A0.transpose()*A0;
  for (int i = 0; i < m0; i++) {
    for (int j = 0; j < m0; j++) {
      P->add(left + i, left + j, (*m0Scratch)(i, j));
    }
  }
}


template <typename T>
class BandedDenseNormalEqs {
public:
  using Mat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
  using Vec = Eigen::Matrix<T, Eigen::Dynamic, 1>;

  BandedDenseNormalEqs(int M0,    // Total number of band variables
                       int m0,    // Max band block width
                       int m1) :  // Dense width
    _M0(M0), _m0(m0), _m1(m1), _kd(m0-1) {
    reset();
  }

  void reset() {
    _P = SymmetricBandMatrixL<T>::zero(_M0, _kd);
    _QR = Mat::Zero(_M0, 1 + _m1);
    _S = Mat::Zero(_m1, _m1);
    _T = Mat::Zero(_m1, 1);
    _U = Mat::Zero(_m1, _M0);
    _m0Scratch = Mat::zero(_m0, _m0);
  }

  template <typename Map>
  void add(int left,     // Band left offset
           Map F,
           Map A0,     // Band part
           Map A1) {   // Dense part
    int m0 = A0.cols();
    CHECK(m0 <= _m0);
    CHECK(A1.cols() == _m1);
    CHECK(F.cols() == 1);
    int rows = F.rows();
    CHECK(rows == A0.rows());
    CHECK(rows == A1.rows());

    addToDiag(left, A0, &_m0Scratch, &_P);

    Q(left, m0) -= A0.transpose()*F; // B = -F
    R(left, m0) -= A0.tranpose()*A1;

    _S += A1.transpose()*A1;
    _T -= A1.transpose()*F; // B = -F
    _U -= A1.transpose()*A0;
  }

  bool solve(Vec *X0, Vec *X1) {
    if (X0->rows() != _M0) {
      *X0 = Vec(_M0);
    }
    auto QRmat = wrap(_QR);
    if (!Pbsv<T>::apply(&_P, &QRmat)) {
      return false;
    }

    return true;
  }
private:
  int _M0, _m0, _m1, _kd;
  SymmetricBandMatrixL<T> _P; // m0*m0
  Mat _QR, _S, _T, _U, _m0Scratch;
  Vec _X0;

  auto Q(int m0, int left) -> decltype(_QR.block(0, 0, 1, 1)) {
    return _QR.block(left, 0, m0, 1);
  }

  auto R(int m0, int left) -> decltype(_QR.block(0, 0, 1, 1)) {
    return _QR.block(left, 0, m0, _m1);
  }
};




}



#endif /* SERVER_MATH_BANDEDDENSE_H_ */
