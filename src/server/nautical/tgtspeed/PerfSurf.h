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
  /**
   * This is the breadth of the smoothness of the Huber estimator
   * used for fitting the surface. A little a bit of smoothness is
   * necessary of the sake of convergence. A lower value of sigma
   * means a slower convergence, but possibly a better result,
   * requiring more iterations of the algorithm.
   *
   */
  Velocity<double> sigma = 0.1_kn;

  /**
   * A weight for every point will make the surface
   * have some gravity.
   */
  double weightPerPoint = 0.1;

  int iterations = 30;
};

Array<Span<int>> makeWindowsInSpan(int width, Span<int> span);

/**
 * This is the algorithm that optimize the surface.
 * The surface is parameterized by a number of vertices.
 * A vertex is a discrete sample on the surface and
 * is the target speed at that point.
 */
Array<Velocity<double>> optimizePerfSurface(
    const Array<TimedValue<PerfSurfPt>>& samples,
    const Array<Span<int>>& windows,
    const Array<Velocity<double>>& initialSurfaceVertices,
    const PerfSurfSettings& settings);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_PERFSURF_H_ */
