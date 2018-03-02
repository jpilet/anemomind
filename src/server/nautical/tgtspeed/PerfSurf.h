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
#include <server/common/logging.h>
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

class SumConstraint {
public:
  SumConstraint(int n, double sum) : _n(n), _avg(sum/n) {}

  static SumConstraint averageConstraint(int n, double avg) {
    return SumConstraint(n, avg/n);
  }

  int coeffCount() const {return _n - 1;}

  struct Comb {
    WeightedIndex i, j;
    double offset = 0;

    bool isContiguousPair() const {
      return i.index + 1 == j.index;
    }

    std::pair<double*, double*> pointers(double* src) const {
      return {src + i.index, src + j.index};
    }

    template <typename T>
    T eval(T a, T b) const {
      return i.weight*a + j.weight*b + offset;
    }

    template <typename T>
    T eval(const Array<T>& src) const {
      return i.weight*src[i.index] + j.weight*src[j.index] + offset;
    }
  };

  Comb get(int index) const {
    CHECK(0 <= index);
    CHECK(index < _n);
    Comb dst;
    dst.offset = _avg;
    if (index == 0) {
      dst.i = WeightedIndex(0, 1.0);
      dst.j = WeightedIndex(1, 0.0);
    } else if (index == _n-1) {
      dst.i = WeightedIndex(_n-3, 0.0);
      dst.j = WeightedIndex(_n-2, -1.0);
    } else {
      dst.i = WeightedIndex(index-1, -1.0);
      dst.j = WeightedIndex(index, 1.0);
    }
    return dst;
  }

  Array<double> apply(const Array<double>& src) const {
    CHECK(src.size() == coeffCount());
    Array<double> dst(_n);
    for (int i = 0; i < _n; i++) {
      dst[i] = get(i).eval(src);
    }
    return dst;
  }
private:
  int _n = 0;
  double _avg = 0;
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
  double regWeight = 1.0;
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
