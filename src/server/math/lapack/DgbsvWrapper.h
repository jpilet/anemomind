/*
 * DgbsvWrapper.h
 *
 *  Created on: Apr 22, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_LAPACK_DGBSVWRAPPER_H_
#define SERVER_MATH_LAPACK_DGBSVWRAPPER_H_

#include <server/math/lapack/BandMatrix.h>

namespace sail {

// Implements a wrapper around this:
// http://www.netlib.org/lapack/explore-html-3.4.2/d3/d49/group__double_g_bsolve.html
//
// Returns true iff successful
bool easyDgbsvInPlace(BandMatrix<double> *lhs, MDArray2d *rhs);

}

#endif /* SERVER_MATH_LAPACK_DGBSVWRAPPER_H_ */
