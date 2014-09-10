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
 * Filter noisy data into discrete values.
 */
MDArray2i quantFilter(Array<LineKM> binMap,
                        MDArray2d noisyData,
                        double regularization);

} /* namespace mmm */

#endif /* BINFILTER_H_ */
