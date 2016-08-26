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

}



#endif /* SERVER_MATH_EIGENUTILS_H_ */
