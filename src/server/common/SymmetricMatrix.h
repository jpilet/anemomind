/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SYMMETRICMATRIX_H_
#define SYMMETRICMATRIX_H_

#include <cassert>

namespace sail {

/*
 * Utilities for indexing symmetric matrices.
 *
 * We assign indices to the lower triangular part of
 * a symmetric matrix, like this:
 *
 * 0
 * 1 2
 * 3 4 5
 * 6 7 8 9
 * . . . . .
 * . . . . . .
 *
 * The leftmost index of a row n, equals the number
 * of elements in the triangular submatrix above
 * that row.
 *
 */

/*
 * Compute the number of
 * elements required to store a symmetric matrix
 * of size 'dim'
 */
constexpr inline int calcSymmetricMatrixStorageSize(int dim) {
  return (dim*(dim + 1))/2;
}

/*
 * Map a matrix entry (i, j) to an index in the storage.
 * It is required for i >= j.
 */
constexpr inline int calcSymmetricMatrixIndexUnsafe(int i, int j) {
  return calcSymmetricMatrixStorageSize(i) + j;
}

/*
 * Does the same as calcSymmetricMatrixIndexUnsafe, but
 * it is not required that i >= j.
 */
constexpr inline int calcSymmetricMatrixIndex(int i, int j) {
  return (i >= j? calcSymmetricMatrixIndexUnsafe(i, j) :
      calcSymmetricMatrixIndexUnsafe(j, i));
}



}



#endif /* SYMMETRICMATRIX_H_ */
