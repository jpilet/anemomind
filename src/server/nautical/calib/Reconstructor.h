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
#include <server/common/DOMUtils.h>
#include <server/common/TimeMapper.h>
#include <server/nautical/filters/SplineGpsFilter.h>
#include <server/math/SplineUtils.h>
#include <server/nautical/calib/MagHdgCalib2.h>
#include <server/common/RNG.h>
#include <server/nautical/calib/CalibDataChunk.h>

namespace sail {

struct MagHdgSettings {
  bool spreadPlot = false;
  bool fittedSinePlot = false;
  bool angleFitnessPlot = false;
  bool costPlot = false;
  MagHdgCalib2::Settings calibSettings;
  AutoRegSettings regSettings;
};

struct ReconstructionSettings {
  MagHdgSettings magHdgSettings;
};

struct ReconstructedChunk {
  TimeMapper mapper;
  Array<BoatState<double>> states;
};

struct ReconstructionResults {
  Array<ReconstructedChunk> chunks;
};

ReconstructionResults reconstruct(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst,
    RNG *rng);

}

#endif /* SERVER_NAUTICAL_CALIB_RECONSTRUCTOR_H_ */
