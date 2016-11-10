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
#include <server/common/Span.h>

namespace sail {

Array<int> listAllBounds(
    const Array<TimeStamp> &times,
    Duration<double> dur);

Array<TimeStamp> listTimeSpans(
    const Array<TimeStamp> &times,
    const Duration<double> dur);

} /* namespace sail */

#endif /* SERVER_COMMON_TIMEDVALUEUTILS_H_ */
