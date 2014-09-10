/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BINFILTER_H_
#define BINFILTER_H_

#include <server/common/LineKM.h>
#include <server/common/MDArray.h>

namespace sail {

/*
 * Filter noisy data into quantized values.
 * Returns the indices of the quantizes values.
 *
 * In short, for every column j of the returned array,
 * we assign integers that map to floating point values
 * by element-wise application of the 'binMap' array on those integers.
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
 *
 *
 * In order to reduce computational complexity, instead of solving one big dynamic programming problem,
 * we solve a series of smaller dynamic programming problems. In every such dynamic programming problem,
 * we optimize the step towards the solution. The non-discretized problem is convex,
 * and it is our hope that the discretized problem will still converge to something close to optimal.
 * The reason for the above optimization is that the complexity of solving the original dynamic programming
 * problem increases exponentially with the dimension. Suppose that we have 3D-vectors that we would like to
 * fit to a 30x30x30 grid. That would amount to solving a StateAssign with 27000 states, which would be very slow.
 */
MDArray2i quantFilter(Array<LineKM> binMap,
                        MDArray2d noisyData,
                        double regularization);

/*
 * If the above function is still too complicated,
 * approximate the problem by slicing up the data and optimizing chunks.
 */
MDArray2i quantFilterChunked(Array<LineKM> binMap,
    MDArray2d noisyData,
    double regularization,
    int chunkSize);

} /* namespace mmm */

#endif /* BINFILTER_H_ */
