/*
 * BandedDense.h
 *
 *  Created on: 3 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_BANDEDDENSE_H_
#define SERVER_MATH_BANDEDDENSE_H_

#include <server/math/lapack/BandMatrix.h>
#include <server/common/logging.h>

namespace sail {

template <typename T>
class BandedDenseNormalEqs {
public:
  using Mat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

  BandedDenseNormalEqs(int M0,    // Total number of band variables
                       int m0,    // Max band block width
                       int m1) :  // Dense width
    _M0(M0), _m0(m0), _m1(m1), _kd(m0-1) {
    reset();
  }

  void reset() {
    _P = SymmetricBandMatrixL<T>::zero(_M0, _kd);
    _Q = Mat::Zero(_M0, 1);
    _R = Mat::Zero(_M0, _m1);
    _S = Mat::Zero(_m1, _m1);
    _T = Mat::Zero(_m1, 1);
    _U = Mat::Zero(_m1, _M0);
    _m0Scratch = Mat::zero(_m0, _m0);
  }

  template <typename Map>
  void add(int left,     // Band left offset
           Map F,
           Map Jac0,     // Band part
           Map Jac1) {   // Dense part
    int m0 = Jac0.cols();
    CHECK(m0 <= _m0);
    CHECK(Jac1.cols() == _m1);
    CHECK(F.cols() == 1);
    int rows = F.rows();
    CHECK(rows == Jac0.rows());
    CHECK(rows == Jac1.rows());

  }
private:
  int _M0, _m0, _m1, _kd;
  SymmetricBandMatrixL<T> _P; // m0*m0
  Mat _Q, _R, _S, _T, _U, _m0Scratch;
};




}



#endif /* SERVER_MATH_BANDEDDENSE_H_ */
