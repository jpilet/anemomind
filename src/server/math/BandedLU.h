/*
 * BandedLU.h
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_BANDEDLU_H_
#define SERVER_MATH_BANDEDLU_H_

#include <server/math/lapack/BandMatrix.h>
#include <server/common/numerics.h>

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
void swapRows(int bestRow, T *a, int blockSize, int colStep) {
  T *row0 = a;
  T *row1 = a + bestRow;
  for (int i = 0; i < blockSize; i++) {
    std::swap(*row0, *row1);
    row0 += colStep;
    row1 += colStep;
  }
}

template <typename T>
void rowOp(T factor, int cols, int from, int to, int colStep, T *a) {
  T *aSrc = a + from;
  T *aDst = a + to;
  for (int i = 0; i < cols; i++) {
    *aDst += factor*(*aSrc);
    aSrc += colStep;
    aDst += colStep;
  }
}

template <typename T>
bool forwardEliminateSquareBlock(
    int blockSize,
    int bCols,
    int aColStep,
    int bColStep,
    T *a, int offset,
    T *b) {
  int bestRow = findBestRowToPivot(blockSize, a);
  swapRows(bestRow, a, blockSize, aColStep);
  swapRows(bestRow, b, blockSize, bColStep);
  if (fabs(a[0]) <= T(1.0e-12)) {
    return false;
  }
  for (int i = 1; i < blockSize; i++) {
    T factor = -a[i]/a[0];
    if (!isFinite<T>(factor)) {
      return false;
    }
    rowOp(factor, blockSize, 0, i, aColStep, a);
    rowOp(factor, bCols, 0, i, bColStep, b);
  }
  return true;
}

template <typename T>
bool forwardEliminate(BandMatrix<T> *A, MDArray<T, 2> *B) {
  int maxBlockSize = getSquareBlockSize(*A);
  int rowStep = A->verticalStride();
  int aColStep = A->horizontalStride();
  int bCols = B->cols();
  int bColStep = B->getStepAlongDim(1);
  assert(rowStep == 1);
  int n = A->rows();
  for (int offset = 0; offset < n; offset++) {
    int blockSize = std::min(n - offset, maxBlockSize);
    auto *a = A->ptr(offset, offset);
    auto *b = B->getPtrAt({offset, 0});
    if (!forwardEliminateSquareBlock(blockSize,
        bCols,
        aColStep,
        bColStep,
        a, offset,
        b)) {
      return false;
    }
  }
  return true;
}

template <typename T>
void solveInPlace(
    BandMatrix<T> *A, MDArray<T, 2> *B) {
  forwardEliminate(A, B);
  backwardSubstitute(A, B);
}

}
}



#endif /* SERVER_MATH_BANDEDLU_H_ */
