/*
 * Reconstructor.h
 *
 *  Created on: 16 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_RECONSTRUCTOR_H_
#define SERVER_NAUTICAL_CALIB_RECONSTRUCTOR_H_

#include <server/common/TimeStamp.h>
#include <server/nautical/BoatState.h>
#include <server/common/Array.h>
#include <map>
#include <string>
#include <device/anemobox/Channels.h>
#include <server/common/TimedValue.h>
#include <server/nautical/calib/BoatParameters.h>
#include <server/common/DOMUtils.h>
#include <server/common/TimeMapper.h>

namespace sail {



// A CalibDataChunk are measurements that are grouped together
// They are dense without any big gaps.
struct CalibDataChunk {
  Array<TimedValue<GeographicPosition<double>>> filteredPositions;
  Array<BoatState<double>> initialStates;
  TimeMapper timeMapper;

#define MAKE_DATA_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, Array<TimedValue<TYPE>>> HANDLE;
FOREACH_CHANNEL(MAKE_DATA_MAP)
#undef MAKE_DATA_MAP
};

struct ReconstructionSettings {};

struct ReconstructedChunk {
  TimeMapper mapper;
  Array<BoatState<double>> states;
};

struct ReconstructionResults {
  BoatParameters<double> parameters;
  Array<ReconstructedChunk> chunks;
};

ReconstructionResults reconstruct(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst);

}

#endif /* SERVER_NAUTICAL_CALIB_RECONSTRUCTOR_H_ */
