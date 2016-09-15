/*
 * Reconstructor.cpp
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/Reconstructor.h>

namespace sail {
namespace Reconstructor {

namespace {

  template <FunctionCode fcode, DataCode code>
  void initializeFields(const CalibDataChunk &src,
      SensorFunctionSet<fcode, double> *dst) {
    typedef typename
        SensorFunctionType<fcode, double, code>::type FType;
    /*const auto &srcMap =
        *ChannelFieldAccess<code>::template get<CalibDataChunk>(src);
    auto &dstMap =
        *ChannelFieldAccess<code>::template get<FType>(*dst);
    for (const auto &kv: srcMap) {

    }*/
  }

  template <FunctionCode code>
  void initializeSensorSetFromChunk(
      const CalibDataChunk &chunk,
      SensorFunctionSet<code, double> *dst) {
    initializeFields<code, AWA>(chunk, dst);
    initializeFields<code, AWS>(chunk, dst);
    initializeFields<code, MAG_HEADING>(chunk, dst);
    initializeFields<code, WAT_SPEED>(chunk, dst);
  }

  template <FunctionCode code>
  SensorFunctionSet<code, double> initializeSensorSet(
      const Array<CalibDataChunk> &chunks) {
    SensorFunctionSet<code, double> dst;
    for (auto chunk: chunks) {
      initializeSensorSetFromChunk<code>(chunk, &dst);
    }
    return dst;
  }

}

Results reconstruct(
    const Array<CalibDataChunk> &chunks,
    const Settings &settings) {

  auto noiseSet = initializeSensorSet<
      FunctionCode::Noise>(chunks);
  auto distortionSet = initializeSensorSet<
      FunctionCode::Distortion>(chunks);

  return Results();
}

}
}
