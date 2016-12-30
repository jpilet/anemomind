/*
 * GameCalib.cpp
 *
 *  Created on: 30 Dec 2016
 *      Author: jonas
 */

#include <server/nautical/calib/GameCalib.h>
#include <server/math/SplineUtils.h>
#include <server/common/string.h>

namespace sail {
namespace GameCalib {

struct SplineParam {
  CubicBasis basis;
  TimeMapper mapper;
};

SplineParam allocateSpline(const CalibDataChunk &chunk,
    Duration<double> period) {
  auto span = chunk.span();
  auto dur = span.maxv() - span.minv();
  int n = std::max(1, int(round(dur/period)));
  TimeMapper mapper(span.minv(), (1.0/n)*dur, n);
  return SplineParam{CubicBasis(mapper.sampleCount), mapper};
}

Array<SplineParam> allocateSplines(
    const Array<CalibDataChunk> &chunks,
    Duration<double> period,
    DOM::Node *dst) {
  int n = chunks.size();
  Array<SplineParam> params(n);
  auto ul = DOM::makeSubNode(dst, "ul");
  for (int i = 0; i < n; i++) {
    auto p = allocateSpline(chunks[i], period);
    params[i] = p;
    if (dst) {
      DOM::addSubTextNode(&ul, "li",
          stringFormat("Spline basis of %d samples for "
              "a total duration of %s with a "
              "sampling period of %s",
              p.mapper.sampleCount,
              p.mapper.totalDuration().str().c_str(),
              p.mapper.period.str().c_str()));
    }
  }
  return params;
}

void optimize(
    const Array<CalibDataChunk> &chunks,
    const Settings &settings,
    DOM::Node *dst) {
  DOM::addSubTextNode(dst, "h1", "Game based calibration");
  DOM::addSubTextNode(dst, "h2", "Allocate wind splines");
  auto windSplines = allocateSplines(chunks,
      settings.windSamplingPeriod, dst);
  DOM::addSubTextNode(dst, "h2", "Allocate current splines");
  auto currentSplines = allocateSplines(chunks,
      settings.currentSamplingPeriod, dst);
}

}
}
