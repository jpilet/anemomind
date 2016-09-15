/*
 * Reconstructor.h
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 *
 * Performs joint calibration and filtering.
 */

#ifndef SERVER_NAUTICAL_RECONSTRUCTOR_H_
#define SERVER_NAUTICAL_RECONSTRUCTOR_H_

#include <device/anemobox/Dispatcher.h>
#include <ceres/ceres.h>
#include <server/nautical/BoatState.h>
#include <unordered_map>
#include <server/nautical/calib/SensorSet.h>

namespace sail {
namespace Reconstructor {

struct Settings {
  Duration<double> windowSize = Duration<double>::minutes(1.0);
};

// A CalibDataChunk are measurements that are grouped together
struct CalibDataChunk {
  Array<TimedValue<GeographicPosition<double>>> filteredPositions;
#define MAKE_DATA_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, Array<TimedValue<TYPE>>> HANDLE;
FOREACH_CHANNEL(MAKE_DATA_MAP)
#undef MAKE_DATA_MAP
};

class ChannelRef {
  DataCode code;
  std::string sourceName;
};

struct Results {
  SensorNoiseSet<double> sensorNoise;
  SensorDistortionSet<double> sensorDistortion;
};

// Reconstructs the noise and distortion for a
// chunks of data.
Results reconstruct(
    const Array<CalibDataChunk> &chunks,
    const Settings &settings);

}
}

#endif /* SERVER_NAUTICAL_RECONSTRUCTOR_H_ */
