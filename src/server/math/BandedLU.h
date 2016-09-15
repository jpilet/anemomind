/*
 * BandedLU.h
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_BANDEDLU_H_
#define SERVER_MATH_BANDEDLU_H_

#include <server/math/lapack/BandMatrix.h>

namespace sail {
namespace BandedLU {

template <typename T>
bool hasSquareBlocks(const BandMatrix<T> &x) {
  return x.subdiagonalCount() == x.superdiagonalCount();
}

template <typename T>
bool hasValidShape(const BandMatrix<T> &x) {
  return x.isSquare() && hasSquareBlocks(x);
}

template <typename T>
int getSquareBlockSize(const BandMatrix<T> &x) {
  assert(hasSquareBlocks(x));
  return 1 + x.superdiagonalCount();
}

inline int computeBlockCount(int matrixSize, int blockSize) {
  return matrixSize - blockSize + 1;
}

template <typename T>
int getDiagonalBlockCount(const BandMatrix<T> &x) {
  return computeBlockCount(x.rows(), getSquareBlockSize(x));
}

/*template <typename T>
void solveInPlace(
    BandMatrix<T> *A Eigen::Matrix<T, Eigen::Dynamic,
    Eigen::Dynamic> *B) {

}*/

}
}



#endif /* SERVER_MATH_BANDEDLU_H_ */
