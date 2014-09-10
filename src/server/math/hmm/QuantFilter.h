/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BINFILTER_H_
#define BINFILTER_H_

#include <server/common/LineKM.h>
#include <server/common/MDArray.h>

namespace sail {

/*
 * Filter noisy data into quantized values.
 * Returns the indices of the quantices values.
 *
 * In short, for every column j of the returned array,
 * we assign integers that map to floating point values
 * by elementwise application of the 'binMap' array on those integers.
 *
 * The integers are optimized so that the least-squared error
 * between the mapped vector and the corresponding column in 'noisyData' is
 * minimized, while regularizing with the norm between the two vectors
 * of two consecutive columns. It can be seen as a generalization of
 * TV-regularization to a one-dimensional discrete signal of vectors instead of scalars,
 * but with the difference that the output filtered vectors are discrete.
 *
 * See test cases for a better idea of how it works.
 *
 * Use case: To identify episodes of navigational data where TWS, TWA and boat speed are jointly constant,
 * while at the same time filtering out noise.
 */
MDArray2i quantFilter(Array<LineKM> binMap,
                        MDArray2d noisyData,
                        double regularization);

} /* namespace mmm */

#endif /* BINFILTER_H_ */
