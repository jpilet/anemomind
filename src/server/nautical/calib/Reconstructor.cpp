/*
 * Reconstructor.cpp
 *
 *  Created on: 16 Oct 2016
 *      Author: jonas
 */

#include <server/nautical/calib/Reconstructor.h>
#include <server/plot/AxisTicks.h>
#include <server/plot/PlotUtils.h>
#include <server/plot/CairoUtils.h>

namespace sail {

ReconstructionResults reconstruct(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst) {
  return ReconstructionResults();
}

}
