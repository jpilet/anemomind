/*
 * CalibDataChunk.h
 *
 *  Created on: 30 Dec 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_CALIBDATACHUNK_H_
#define SERVER_NAUTICAL_CALIB_CALIBDATACHUNK_H_

#include <device/anemobox/Channels.h>
#include <server/common/TimedValue.h>
#include <server/nautical/filters/SplineGpsFilter.h>

namespace sail {

// A CalibDataChunk are measurements that are grouped together
// They are dense without any big gaps.
struct CalibDataChunk {
  SplineGpsFilter::EcefCurve trajectory;
  Span<TimeStamp> span() const {return trajectory.span();}

#define MAKE_DATA_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, Array<TimedValue<TYPE>>> HANDLE;
FOREACH_CHANNEL(MAKE_DATA_MAP)
#undef MAKE_DATA_MAP
};

}

#endif /* SERVER_NAUTICAL_CALIB_CALIBDATACHUNK_H_ */
