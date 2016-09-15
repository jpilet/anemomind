/*
 * BandedLU.h
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_BANDEDLU_H_
#define SERVER_MATH_BANDEDLU_H_

#include <Eigen/Dense>
#include <server/math/lapack/BandMatrix.h>

namespace sail {
namespace BandedLU {

template <typename T>
using DenseMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

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

template <typename T>
class SteppedArray {
public:
  SteppedArray(T *data, int step) : _data(data), _step(step) {}
  T &operator[](int i) {return _data[i*_step];}
  const T &operator[](int i) const {return _data[i*_step];}
private:
  T *_data;
  int _step;
};

template <typename T>
int findBestRowToPivot(int n, T *a) {
  int bestIndex = 0;
  T bestValue = a[0];
  for (int i = 1; i < n; i++) {
    auto value = fabs(a[i]);
    if (bestValue < value) {
      bestValue = value;
      bestIndex = i;
    }
  }
  return bestIndex;
}

template <typename T>
void forwardEliminateSquareBlock(
    int blockSize,
    int colStep,
    T *a, int offset,
    DenseMat<T> *B) {
  int bestRow = findBestRowToPivot(blockSize, a);
}

template <typename T>
bool forwardEliminate(BandMatrix<T> *A, DenseMat<T> *B) {
  int maxBlockSize = getSquareBlockSize(*A);
  int rowStep = A->verticalStride();
  int colStep = A->horizontalStride();
  assert(rowStep == 1);
  int n = A->rows();
  for (int offset = 0; offset < n; offset++) {
    int blockSize = std::min(n - offset, maxBlockSize);
    auto *a = A->ptr(offset, offset);
    if (!forwardEliminateBlock(blockSize, colStep, a, offset, B)) {
      return false;
    }
  }
  return true;
}

template <typename T>
void solveInPlace(
    BandMatrix<T> *A,
    DenseMat<T> *B) {
  //forwardEliminate();
}

}
}



#endif /* SERVER_MATH_BANDEDLU_H_ */
