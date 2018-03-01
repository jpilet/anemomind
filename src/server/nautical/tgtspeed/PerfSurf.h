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
#include <Eigen/Dense>

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

enum SystemConstraintType {
  Norm1,
  Sum1
};

// Minimize |AX|^2 subject to either sum(X)=1 or |X|=1
// Input argument is the matrix A'*A
Eigen::VectorXd solveConstrained(
    const Eigen::MatrixXd& AtA,
    SystemConstraintType type);

struct PerfSurfSettings {
  std::function<Velocity<double>(PerfSurfPt)> refSpeed;
  double maxFactor = 4.0;
  int iterations = 10;

  double temporalReg = 1.0;
  double surfaceReg = 1.0;
};

Array<std::pair<int, int>> generatePairs(const Array<Spani>& spans, int step);

Array<std::pair<int, int>> generateSurfaceNeighbors1d(int vertexCount);

Eigen::MatrixXd makeOneDimensionalReg(int n, int order);

// Preprocessed data point. Easier to chew.
struct PerfFitPoint {

  // Its time index
  int index = 0;

  // The speed divided by reference speed
  double normedSpeed = 0.0;

  // Where it is on the surface
  Array<WeightedIndex> weights;

  // If it is good.
  bool good = false;

  PerfFitPoint evaluateLevel(double* levelData) const;
};

struct PerfSurfResults {
  Array<double> rawNormalizedVertices;
  Array<double> rawPerformances;
};

PerfSurfResults optimizePerfSurf(
    const Array<PerfSurfPt>& pts,
    const Array<std::pair<int, int>>& surfaceNeighbors,
    const PerfSurfSettings& settings);



} /* namespace sail */

#endif /* SERVER_NAUTICAL_PERFSURF_H_ */
