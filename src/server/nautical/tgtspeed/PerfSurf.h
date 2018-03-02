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
#include <ceres/loss_function.h>

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

inline std::ostream& operator<<(std::ostream& s, const WeightedIndex& wi) {
  s << "(i=" << wi.index << ", w=" << wi.weight << ")";
  return s;
}

// Used to implement parameterization of
// a vector that is linearly constrained
// so that its coefficients sum up to a cerain
// value.
class SumConstraint {
public:
  SumConstraint(int n, double sum) : _n(n), _avg(sum/n) {}

  static SumConstraint averageConstraint(int n, double avg) {
    return SumConstraint(n, avg*n);
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
  // Time, if applicable
  TimeStamp time;

  // Representation of the wind at this point as a linear combination
  // of the winds at the vertices.
  Array<WeightedIndex> windVertexWeights;

  // Measured speed at that point.
  Velocity<double> boatSpeed;

  // Should be in the interval [0, 1].
  // Used only for testing, to store the ground truth.
  double performance = 0.0;

  // Used to group points together in time.
  int groupIndex = 0;
};

struct PerfSurfSettings {

  // All speeds are divided by this function
  // to bring them to a normalized domain. To
  // go back from the normalized domain, we multiply
  // by it.
  std::function<Velocity<double>(Array<WeightedIndex>)> refSpeed;

  // The highest allowed value in the normalized domain.
  double maxFactor = 4.0;

  // Control the temporal smoothness
  // of the fitted performances.
  double perfRegWeight = 1.0;

  // Control the smoothness of the
  // fitted target speed surface
  double vertexRegWeight = 1.0;

  // Where we start looking for
  // a reference quantile of the surface.
  double surfaceQuantile = 0.9;

  // Parameter for the post-scaling of
  // the solution. Not so important.
  double quantileMarg = 0.1;

  // Squared loss by default.
  ceres::LossFunction* dataLoss = nullptr;
};

Array<std::pair<int, int>> generateSurfaceNeighbors1d(int vertexCount);

Eigen::MatrixXd makeOneDimensionalReg(int n, int order);

// Preprocessed data point. Easier to chew.
struct PerfFitPoint {

  // Its time index
  int index = 0;

  // The speed divided by reference speed
  double normedSpeed = 0.0;

  // The wind of this point, expressed as a linear combination
  // of the wind at the vertices of the surface that we are fitting.
  Array<WeightedIndex> weights;

  // If it is good. An example of a bad point is a point
  // with a very high or infinite normedSpeed. That can happen
  // due to division by a small reference speed.
  bool good = false;

  // Which group in time that this point belongs to.
  // Used to apply regularization where we should.
  int groupIndex = 0;

  PerfFitPoint evaluateLevel(double* levelData) const;
};

struct RawPerfSurfResults {
  // Raw optimized vertices in normalized domain.
  // They need to be scaled by a factor determined
  // in the post processing step.
  Array<double> rawNormalizedVertices;

  // Raw performance values associated with
  // the input data points. They need to be
  // scaled before they are useful.
  Array<double> rawPerformances;
};

// This function performs the actual target
// speed surface optimization, but its output is
// pretty much the raw output of the optimizer.
//
// Please see the 'optimizePerfSurf' function.
RawPerfSurfResults optimizePerfSurfSub(
    const Array<PerfSurfPt>& pts,
        const Array<std::pair<int, int>>& surfaceNeighbors,
        const PerfSurfSettings& settings);

// Implements heuristic used when
// scaling the solution in the postprocessing step.
double binarySearchPerfThreshold(
    const Array<double>& sortedRawPerfs,
    double startQuantile);

struct PerfSurfResults {
  Array<double> performances;

  // Correctly scaled, but normalized vertices.
  Array<double> normalizedVertices;

  // This is the target speed at every vertex.
  // What we are interested in.
  Array<Velocity<double>> vertices;
};

// The top-level function for the target
// speed optimization algorithm.
//
// It can be used for any kind of surface, such as a 1d line
// or a full polar surface.
PerfSurfResults optimizePerfSurf(
    const Array<PerfSurfPt>& pts,
    const Array<std::pair<int, int>>& surfaceNeighbors,
    const PerfSurfSettings& settings);


} /* namespace sail */

#endif /* SERVER_NAUTICAL_PERFSURF_H_ */
