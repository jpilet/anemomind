/*
 * PerfSurf.h
 *
 *  Created on: 12 Nov 2017
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_PERFSURF_H_
#define SERVER_NAUTICAL_PERFSURF_H_

#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/Span.h>
#include <server/common/TimedValue.h>
#include <ceres/jet.h>

namespace sail {

/*
 * A weighted index is a struct used to encoding a sparse linear combination
 * of vertices in order to parameterize a point. Barycentric coordinates in
 * computer graphics are a special case of that.
 */
struct WeightedIndex {
  int index = 0;
  double weight = 1.0;

  WeightedIndex() {}
  WeightedIndex(int i, double w) : index(i), weight(w) {}
};

/*
 * This is the format of the input data to the algorithm. The input data
 * are lots of samples of wind w.r.t boat and the boat speed.
 *
 * There is also a 'performance' value that might, or might not,
 * be used internally in the algorithm. No need to assign that
 * value to anything inside the algorithm.
 */
struct PerfSurfPt {
  TimeStamp time; // Time, if applicable
  Array<WeightedIndex> windVertexWeights; // Representation of the point on perf surface
  Velocity<double> boatSpeed; // Measured speed at that point
  double performance = 0.0; // Should be in the interval [0, 1]
};

struct PerfSurfSettings {
  std::function<Velocity<double>(PerfSurfPt)> refSpeed;
  double maxFactor = 4.0;
};

Array<Span<int>> makeWindowsInSpan(int width, Span<int> span);

ceres::Jet<double, 1> evaluateHuber(double x0, double sigma0);

/**
 * This is the algorithm that optimize the surface.
 * The surface is parameterized by a number of vertices.
 * A vertex is a discrete sample on the surface and
 * is the target speed at that point.
 */
Array<Array<Velocity<double>>> optimizePerfSurface(
    const Array<PerfSurfPt>& samples,
    const Array<Span<int>>& windows,
    const Array<Array<WeightedIndex>>& regTerms,
    const Array<Velocity<double>>& initialSurfaceVertices,
    const PerfSurfSettings& settings);

Array<Velocity<double>> optimizePerfSurfaceHomogeneous(
    const Array<PerfSurfPt>& samples,
    int vertexCount,
    double regWeight);




} /* namespace sail */

#endif /* SERVER_NAUTICAL_PERFSURF_H_ */
