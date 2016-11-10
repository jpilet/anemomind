/*
 * TimedValueUtils.h
 *
 *  Created on: 10 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TIMEDVALUEUTILS_H_
#define SERVER_COMMON_TIMEDVALUEUTILS_H_

#include <server/common/TimeStamp.h>
#include <server/common/Array.h>

namespace sail {

Array<int> listAllBounds(const Array<TimeStamp> &times);

} /* namespace sail */

#endif /* SERVER_COMMON_TIMEDVALUEUTILS_H_ */
