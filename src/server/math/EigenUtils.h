/*
 * EigenUtils.h
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_EIGENUTILS_H_
#define SERVER_MATH_EIGENUTILS_H_

namespace sail {

template <typename T, int M, int N>
bool isFinite(const Eigen::Matrix<T, M, N> &m) {
  for (int i = 0; i < m.rows(); i++) {
    for (int j = 0; j < m.cols(); j++) {
      if (!isFinite(m(i, j))) {
        return false;
      }
    }
  }
  return true;
}

template <typename T, int rows, int cols>
bool isZero(const Eigen::Matrix<T, rows, cols> &M, T tol) {
  for (int i = 0; i < M.rows(); i++) {
    for (int j = 0; j < M.cols(); j++) {
      T x = M(i, j);
      if (!(-tol < x && x < tol)) {
        return false;
      }
    }
  }
  return true;
}

}



#endif /* SERVER_MATH_EIGENUTILS_H_ */
