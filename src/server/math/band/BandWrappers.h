/*
 * BandedWrappers.h
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include <server/math/band/BandMatrix.h>

#ifndef SERVER_MATH_LAPACK_BANDEDWRAPPERS_H_
#define SERVER_MATH_LAPACK_BANDEDWRAPPERS_H_

namespace sail {

template <typename T>
struct Pbsv {
 // Solves a linear system where the left-hand-side (lhs) is a positive
 // definite symmetric matrix. Nothing is assumed about the contents of
 // 'lhs' after it has been solved: it could just be garbage. The 'rhs'
 // on the other hand will be overwritten by the solution. Returns true
 // if successful.
 static bool apply(SymmetricBandMatrixL<T> *lhs, MDArray<T, 2> *rhs);
};

template <typename T>
bool easyGbsv(BandMatrix<T> *lhs, MDArray<T, 2> *rhs);

} /* namespace sail */

#endif /* SERVER_MATH_LAPACK_BANDEDWRAPPERS_H_ */
