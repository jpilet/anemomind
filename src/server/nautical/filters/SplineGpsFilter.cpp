/*
 * SplineGpsFilter.cpp
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */

#include "SplineGpsFilter.h"
#include <server/common/ArrayBuilder.h>

namespace sail {
namespace SplineGpsFilter {


Array<Curve> allocateCurves(const Array<TimeMapper> &mappers) {
  int n = mappers.size();
  Array<Curve> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = Curve(mappers[i]);
  }
  return dst;
}

int computeTotalSampleCount(const Array<TimeMapper> &mappers) {
  int n = 0;
  for (auto m: mappers) {
    n += m.sampleCount;
  }
  return n;
}

Array<Span<int>> listSampleSpans(const Array<Curve> &curves) {
  int n = curves.size();
  Array<Span<int>> dst(n);
  int offset = 0;
  for (int i = 0; i < curves.size(); i++) {
    int next = offset + curves[i].timeMapper.sampleCount;
    dst[i] = Span<int>(offset, next);
    offset = next;
  }
  return dst;
}

Array<Span<TimeStamp>> listTimeSpans(const Array<Curve> &curves) {
  int n = curves.size();
  Array<Span<TimeStamp>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = curves[i].timeSpan();
  }
  return dst;
}

template <typename T>
Array<Array<TimedValue<T>>> segmentTimedValues(
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<TimedValue<T>> &values) {
  int n = timeSpans.size();
  int current = 0;
  Array<ArrayBuilder<TimedValue<T>>> dst(n);
  for (auto sample: values) {
    if (timeSpans.size() <= current) {
      break;
    }

    while (current < timeSpans.size() &&
        timeSpans[current].maxv() < sample.time) {
      current++;
    }
    if (timeSpans[current].contains(sample.time)) {
      dst[current].add(current);
    }
  }
  Array<Array<TimedValue<T>>> dst2(n);
  for (int i = 0; i < n; i++) {
    dst2[i] = dst[i].get();
  }
  return dst2;
}

void buildProblem(
    const Array<Curve> &curves,
    const Array<Span<int>> &sampleSpans,
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    BandedLevMar::Problem<double> *dst) {
  auto pd = segmentTimedValues<GeographicPosition<double>>(
      timeSpans, positionData);
  auto md = segmentTimedValues<HorizontalMotion<double>>(
      timeSpans, motionData);
  for (int i = 0; i < curves.size(); i++) {
    /*buildProblemPerCurve(sampleSpans[i],
        curves[i],
        positionData,
        motionData);*/
  }
}

Array<Curve> filter(
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    const Array<TimeMapper> &segments,
    const Settings &settings) {
  auto curves = allocateCurves(segments);
  int totalSampleCount = computeTotalSampleCount(segments);
  int dim = 4.0*totalSampleCount;

  BandedLevMar::Settings lmSettings;
  BandedLevMar::Problem<double> problem;
  auto X = Eigen::VectorXd::Zero(dim);
  auto sampleSpans = listSampleSpans(curves);
  CHECK(sampleSpans.size() == curves.size());
  CHECK(sampleSpans.last().maxv() == totalSampleCount);
  auto timeSpans = listTimeSpans(curves);
  buildProblem(
      curves, sampleSpans,timeSpans,
      positionData, motionData, &problem);

  return curves;
}



}
}
