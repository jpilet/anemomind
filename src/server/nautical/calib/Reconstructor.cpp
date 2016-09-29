/*
 * Reconstructor.cpp
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/calib/Reconstructor.h>

namespace sail {
namespace Reconstructor {



void Results::outputSummary(std::ostream *dst) const {
  *dst << "Distortion:\n";
  sensorDistortion.outputSummary(dst);
  *dst << "Noise:\n";
  sensorNoise.outputSummary(dst);
}

Results reconstruct(
    const Array<CalibDataChunk> &chunks,
    const Settings &settings) {

  auto noiseSet = initializeSensorSet<
      FunctionCode::Noise>(chunks);
  auto distortionSet = initializeSensorSet<
      FunctionCode::Distortion>(chunks);

  return Results{noiseSet, distortionSet};
}

}
}
