/*
 * BandedWrappers.h
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include <server/math/lapack/BandMatrix.h>

#ifndef SERVER_MATH_LAPACK_BANDEDWRAPPERS_H_
#define SERVER_MATH_LAPACK_BANDEDWRAPPERS_H_

namespace sail {

template <typename T>
bool easyPbsv(SymmetricBandMatrixL<T> *lhs, MDArray<T, 2> *rhs);


bool easyDgbsvInPlace(BandMatrix<double> *lhs, MDArray2d *rhs);

} /* namespace sail */

#endif /* SERVER_MATH_LAPACK_BANDEDWRAPPERS_H_ */
