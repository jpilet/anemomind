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
#include <server/common/logging.h>

// http://www.boost.org/doc/libs/1_61_0/libs/numeric/ublas/doc/banded.html
// http://math.nist.gov/lapack++/

namespace sail {
namespace BandedLU {

template <typename T>
int getSymmetricBandWidth(const BandMatrix<T> &x) {
  return x.subdiagonalCount();
}

template <typename T>
bool hasExtraPivotingSpace(const BandMatrix<T> &A) {
  return A.subdiagonalCount()+1 == A.superdiagonalCount();
}

template <typename T>
bool isDiagonal(const BandMatrix<T> &A) {
  return A.subdiagonalCount() == 0 && A.superdiagonalCount() == 0;
}

template <typename T>
bool hasValidShape(const BandMatrix<T> &x) {
  int l = x.subdiagonalCount();
  int u = x.superdiagonalCount();
  return x.isSquare() &&
      (isDiagonal(x) || hasExtraPivotingSpace(x));
}

template <typename T>
bool pivotingSpaceIsZero(const BandMatrix<T> &A) {
  int k = A.superdiagonalCount();
  for (int i = k; i < A.cols(); i++) {
    if (!(A(i-k, i) == T(0.0))) {
      return false;
    }
  }
  return true;
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
  T bestValue = fabs(a[0]);
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
    int blockRows,
    int blockCols,
    int bCols,
    int aColStep,
    int bColStep,
    T *a, int offset,
    T *b) {
  int bestRow = findBestRowToPivot(blockRows, a);
  std::cout << "  bestRow = " << bestRow << std::endl;
  std::cout << "  blockRows = " << blockRows << std::endl;
  std::cout << "  blockCols = " << blockCols << std::endl;
  std::cout << "  aColStep = " << aColStep << std::endl;
  std::cout << "  bColStep = " << bColStep << std::endl;
  std::cout << "  bCols = " << bCols << std::endl;
  std::cout << "  offset = " << offset << std::endl;
  //swapRows(bestRow, a, blockCols, aColStep);
  //swapRows(bestRow, b, bCols, bColStep);
  if (fabs(a[0]) <= T(1.0e-12)) {
    return false;
  }
  std::cout << "performed swap" << std::endl;
  std::cout << "The diagonal element is now" << a[0] << std::endl;
  for (int i = 1; i < blockRows; i++) {
    T factor = -a[i]/a[0];
    std::cout << "The factor is " << factor << std::endl;
    if (!isFinite<T>(factor)) {
      return false;
    }
    rowOp(factor, blockCols, 0, i, aColStep, a);
    rowOp(factor, bCols, 0, i, bColStep, b);
  }
  std::cout << "all good!" << std::endl;
  return true;
}

template <typename T>
T *getBRowPointer(MDArray<T, 2> *B, int row) {
  int binds[2] = {row, 0};
  return B->getPtrAt(binds);
}

template <typename T>
int getMaxBlockRows(const BandMatrix<T> &A) {
  return 1 + getSymmetricBandWidth(A);
}

template <typename T>
MDArray<T, 2> collectCoefficients(int rows, int cols, T *a, T *b, int aColStep) {
  MDArray<T, 2> dst(rows, cols+1);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      dst(i, j) = a[i + aColStep*j];
    }
    dst(i, cols) = b[i];;
  }
  return dst;
}

template <typename T>
bool forwardEliminate(BandMatrix<T> *A, MDArray<T, 2> *B, int upto = -1) {
  CHECK(hasValidShape(*A));
  if (isDiagonal(*A)) {
    return true;
  }
  assert(pivotingSpaceIsZero(*A));

  int maxBlockRows = getMaxBlockRows(*A);
  int maxBlockCols = 1 + maxBlockRows;

  std::cout << "Max block rows = " << maxBlockRows << std::endl;
  std::cout << "Max block cols = " << maxBlockCols << std::endl;

  assert(1 == A->verticalStride());
  assert(1 == B->getStepAlongDim(0));
  int aColStep = A->horizontalStride();
  int bCols = B->cols();
  int bColStep = B->getStepAlongDim(1);
  int n = A->rows();
  for (int offset = 0; offset < (upto == -1? n : std::min(n, upto)); offset++) {
    int blockRows = std::min(n - offset, maxBlockRows);
    int blockCols = std::min(n - offset, maxBlockCols);
    auto *a = A->ptr(offset, offset);
    auto *b = getBRowPointer(B, offset);
    //std::cout << "Coefficitiones before: " << collectCoefficients(
        //blockRows, blockCols, a, b, aColStep) << std::endl;
    if (!forwardEliminateSquareBlock(
        blockRows,
        blockCols,
        bCols,
        aColStep,
        bColStep,
        a, offset,
        b)) {
      return false;
    }
    //std::cout << "Coefficitiones after: " << collectCoefficients(
        //blockRows, blockCols, a, b, aColStep) << std::endl;
    std::cout << "Forward eliminated at " << offset << ": " << std::endl;
    std::cout << A->makeDense() << std::endl;
    std::cout << "B = \n" << *B << std::endl;
  }
  return true;
}

template <typename T>
bool solveVariable(T a, T *b, int bCols, int bColStep) {
  for (int i = 0; i < bCols; i++) {
    b[0] /= a;
    b += bColStep;
  }
  return true;
}

template <typename T>
bool backwardSubstituteSquareBlock(int backSteps, int bCols,
    int bColStep, T *a, T *b) {
  assert(T(0.0) < fabs(*a));
  solveVariable<T>(*a, b, bCols, bColStep);
  *a = T(1.0);
  for (int i = 1; i < backSteps; i++) {
    T factor = -a[-i];
    a[-i] = T(0.0);
    rowOp(factor, bCols, 0, -i, bColStep, b);
  }
  return true;
}

template <typename T>
bool backwardSubstitute(BandMatrix<T> *A, MDArray<T, 2> *B) {
  int n = A->rows();
  int maxBlockSize = getMaxBlockRows(*A);
  int rowStep = A->verticalStride();
  int bCols = B->cols();
  int aColStep = A->horizontalStride();
  int bColStep = B->getStepAlongDim(1);
  for (int offset = n-1; 0 <= offset; offset--) {
    int from = std::max(0, offset - A->superdiagonalCount());
    int backSteps = offset - from + 1;
    auto *a = A->ptr(offset, offset);
    auto *b = getBRowPointer(B, offset);
    if (!backwardSubstituteSquareBlock<T>(
        backSteps, bCols, bColStep, a, b)) {
      return false;
    }
  }
  return true;
}

template <typename T>
bool solveInPlace(
    BandMatrix<T> *A, MDArray<T, 2> *B) {
  assert(hasValidShape(*A));

  std::cout << " A0 = [" << A->makeDense() << "];\n";
  std::cout << " B0 = [" << *B << "];\n";

  if (!forwardEliminate(A, B)) {
    return false;
  }

  std::cout << " A1 = [" << A->makeDense() << "];\n";
  std::cout << " B1 = [" << *B << "];\n";


  if (!backwardSubstitute(A, B)) {
    return false;
  }
  return true;
}

}
}



#endif /* SERVER_MATH_BANDEDLU_H_ */
