/*
 * PerfSurf.cpp
 *
 *  Created on: 12 Nov 2017
 *      Author: jonas
 */

#include <server/nautical/tgtspeed/PerfSurf.h>
#include <server/common/logging.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Span.h>

namespace sail {

Array<Span<int>> makeWindowsInSpan(int width, Span<int> span) {
  int step = width/2;
  LOG(INFO) << "Window step: " << step;
  int n = span.width()/step + 3;
  ArrayBuilder<Span<int>> windows(n);
  for (int i = span.minv(); i + width <= span.maxv(); i += step) {
    windows.add(Span<int>(i, i+width));
  }
  auto result = windows.get();
  CHECK(result.empty() || result.last().maxv() <= span.maxv());
  return result;
}

Array<Velocity<double>> iterateSurfaceVertices(
    const Array<TimedValue<PerfSurfPt>>& samples,
    const Array<Span<int>>& windows,
    const Array<Velocity<double>>& surfaceVertices,
    const PerfSurfSettings& settings) {
  return surfaceVertices;
}


Array<Velocity<double>> optimizePerfSurface(
    const Array<TimedValue<PerfSurfPt>>& samples0,
    const Array<Span<int>>& windows,
    const Array<Velocity<double>>& initialSurfaceVertices,
    const PerfSurfSettings& settings) {
  auto samples = samples0.dup();
  auto surfaceVertices = initialSurfaceVertices.dup();
  for (int i = 0; i < settings.iterations; i++) {
    surfaceVertices = iterateSurfaceVertices(
        samples, windows, surfaceVertices,
        settings);
  }
  return surfaceVertices;
}

} /* namespace sail */
