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
#include <server/common/math.h>
#include <Eigen/SparseCholesky>
#include <Eigen/EigenValues>
#include <server/math/Majorize.h>
#include <ceres/jet.h>
#include <array>
#include <ceres/ceres.h>

namespace sail {

Velocity<double> evaluateSurfaceSpeed(
    const Array<WeightedIndex>& weights,
    const Array<Velocity<double>>& vertices) {
  Velocity<double> v = 0.0_kn;
  for (const auto& w: weights) {
    v += w.weight*vertices[w.index];
  }
  return v;
}

Array<std::pair<int, int>> generateSurfaceNeighbors1d(int vertexCount) {
  int pairCount = vertexCount-1;
  Array<std::pair<int, int>> dst(pairCount);
  for (int i = 0; i < pairCount; i++) {
    dst[i] = {i, i+1};
  }
  return dst;
}

Array<WeightedIndex> scaleWeights(double s, const Array<WeightedIndex>& src) {
  int n = src.size();
  Array<WeightedIndex> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = src[i];
    dst[i].weight *= s;
  }
  return dst;
}

PerfFitPoint makePerfFitPoint(
    const Array<PerfSurfPt>& data,
    int index, const PerfSurfSettings& s) {
  const auto& x = data[index];
  PerfFitPoint pt;
  pt.index = index;
  pt.normedSpeed = x.boatSpeed/s.refSpeed(x.windVertexWeights);
  pt.weights = x.windVertexWeights;
  pt.good = std::isfinite(pt.normedSpeed) && pt.normedSpeed < s.maxFactor;
  pt.groupIndex = x.groupIndex;
  return pt;
}

Array<PerfFitPoint> preprocessData(
    const Array<PerfSurfPt>& pts,
    const PerfSurfSettings& s) {
  int n = pts.size();
  Array<PerfFitPoint> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = makePerfFitPoint(pts, i, s);
  }
  return dst;
}

void populateMap(
    const Array<WeightedIndex>& src,
    std::map<int, double>* dst) {
  for (auto w: src) {
    auto f = dst->find(w.index);
    if (f == dst->end()) {
      (*dst)[w.index] = w.weight;
    } else {
      f->second += w.weight;
    }
  }
}

Array<WeightedIndex> toWeightedInds(const std::map<int, double>& src) {
  int n = src.size();
  ArrayBuilder<WeightedIndex> dst;
  for (auto kv: src) {
    dst.add({kv.first, kv.second});
  }
  return dst.get();
}

Array<WeightedIndex> add(
    const Array<WeightedIndex>& a,
    const Array<WeightedIndex>& b) {
  std::map<int, double> m;
  populateMap(a, &m);
  populateMap(b, &m);
  return toWeightedInds(m);
}


int getVertexCount(
    const Array<std::pair<int, int>>& surfaceNeighbors) {
  int maxVertex = 0;
  for (const auto& x: surfaceNeighbors) {
    maxVertex = std::max(maxVertex, std::max(x.first, x.second));
  }
  return maxVertex+1;
}

namespace {

  constexpr int SurfaceSegmentDim = 3;


  struct DataFitCost {

    PerfFitPoint pt;
    SumConstraint::Comb cst;

    static ceres::CostFunction* make(
        const PerfFitPoint& p,
        const SumConstraint::Comb& cst) {
      return new ceres::AutoDiffCostFunction<DataFitCost, 1, 1, 1, 1, 1, 1>(
          new DataFitCost{p, cst});
    }

    template <typename T>
    bool operator()(
        const T* v0, // Vertices
        const T* v1,
        const T* v2,
        const T* p0,
        const T* p1,
        T* residual) const {
      typedef const T* Ptr;
      Ptr v[SurfaceSegmentDim] = {v0, v1, v2};
      CHECK(pt.weights.size() <= SurfaceSegmentDim);
      T perf = cst.eval(*p0, *p1);
      T interpolatedTargetSpeed = T(0.0);

      // Only loop over values that we use.
      for (int i = 0; i < pt.weights.size(); i++) {
        interpolatedTargetSpeed += *(v[i])*pt.weights[i].weight;
      }
      *residual =
          perf*interpolatedTargetSpeed // <-- estimated boat speed
          - pt.normedSpeed;            // <-- observed boat speed

      return true;
    }
  };

  std::array<double*, 3> getPointers(
      const std::array<WeightedIndex, 3>& x,
      Array<double>& src) {
    std::array<double*, 3> dst;
    for (int i = 0; i < 3; i++) {
      dst[i] = &(src[x[i].index]);
    }
    return dst;
  }

  typedef std::array<WeightedIndex, 3> WeightedInds3;

  struct ParamRegCost {
    double weight = 1.0;
    WeightedInds3 wi;

    static ceres::CostFunction* make(
        double w,
        const WeightedInds3& src) {
      auto cost = new ParamRegCost();
      cost->weight = w;
      cost->wi = src;
      return new ceres::AutoDiffCostFunction<ParamRegCost, 1, 1, 1, 1>(
          cost);
    }

    template <typename T>
    bool operator()(
        const T* a0, const T* a1, const T* a2,

        T* residual) const {
      *residual = weight*(
          *a0*wi[0].weight
          + *a1*wi[1].weight
          + *a2*wi[2].weight);
      return true;
    }
  };

  struct RegCost {
    double weight = 1.0;

    static ceres::CostFunction* make(
        double w) {
      auto cost = new RegCost();
      cost->weight = w;
      return new ceres::AutoDiffCostFunction<RegCost, 1, 1, 1>(
          cost);
    }

    template <typename T>
    bool operator()(
        const T* a0,
        const T* b0,
        T* residual) const {
      *residual = weight*(*a0 - *b0);
      return true;
    }
  };

  void insertIfValid(std::set<int>* dst, int n, int i) {
    if (0 <= i && i < n) {
      dst->insert(i);
    }
  }

  std::array<double*, SurfaceSegmentDim> getVertexPointers(
      const Array<WeightedIndex>& weights,
      Array<double>& arr) {
    std::array<double*, SurfaceSegmentDim> dst;
    int n = weights.size();
    CHECK(n <= SurfaceSegmentDim);


    int vertexCount = arr.size();

    int extra = SurfaceSegmentDim - n;
    CHECK(0 <= extra && extra <= 1);
    std::set<int> other;
    for (int i = 0; i < n; i++) {
      int vi = weights[i].index;
      CHECK(0 <= vi);
      CHECK(vi < vertexCount);
      dst[i] = &(arr[vi]);
      if (0 < extra) {
        other.insert(vi);
        insertIfValid(&other, vertexCount, vi-1);
        insertIfValid(&other, vertexCount, vi+1);
      }
    }

    for (int i = 0; i < n; i++) {
      other.erase(weights[i].index);
    }

    // Fill the remaining cells with valid, but unused,
    // pointers.
    for (int i = n; i < SurfaceSegmentDim; i++) {
      auto f = other.begin();
      dst[i] = &(arr[*f]);
      other.erase(f);
    }

    // Check again that everything is good
    for (int i = 0; i < SurfaceSegmentDim; i++) {
      auto p = dst[i] - arr.getData();
      CHECK(0 <= p && p < vertexCount);
    }

    return dst;
  }

  struct InitializedDouble {
    double value = 0;
  };

  void accValue(
      std::map<int, InitializedDouble>* dst,
      int sign,
      WeightedIndex i) {
    (*dst)[i.index].value += sign*i.weight;
  }

  std::array<WeightedIndex, 3> combDiff(
      const SumConstraint::Comb& a,
      const SumConstraint::Comb& b) {
    CHECK(a.isContiguousPair());
    CHECK(b.isContiguousPair());

    std::map<int, InitializedDouble> dst;
    accValue(&dst, 1, a.i);
    accValue(&dst, 1, a.j);
    accValue(&dst, -1, b.i);
    accValue(&dst, -1, b.j);


    std::array<WeightedIndex, 3> result;
    {
      int i = 0;
      for (auto x: dst) {
        result[i++] = WeightedIndex(x.first, x.second.value);
      }
      CHECK(i == 2 || i == 3);
      if (i == 2) {
        int fi = result[0].index;
        result[2] = WeightedIndex(fi == 0? 2 : fi-1, 0.0); // Filler
      }
    }

    std::set<int> inds;
    for (int i = 0; i < 3; i++) {
      inds.insert(result[i].index);
    }
    CHECK(inds.size() == 3);

    return result;
  }

}

/*
 *
 *    solve
 *
 *    srcCount*(srcWeight*x)^2 = dstCount*(dstWeight*x)^2
 *
 *    w.r.t. dstWeight
 *
 */
double transferWeight(int srcCount, double srcWeight, int dstCount) {
  CHECK(0 < dstCount);
  return sqrt((srcWeight*srcCount)/dstCount);
}

RawPerfSurfResults optimizePerfSurfSub(
    const Array<PerfSurfPt>& pts,
    const Array<std::pair<int, int>>& surfaceNeighbors,
    const PerfSurfSettings& settings) {
  CHECK(bool(settings.refSpeed));

  int vertexCount = getVertexCount(surfaceNeighbors);

  int pointCount = pts.size();

  // The normalized height of every vertex. The vertices
  // are considered unknown and unconstrained. Start with
  // flat surface in the normalized domain. Note that due to how
  // the objective function is built, starting at 0 might be a local
  // optimimum.
  Array<double> vertices = Array<double>::fill(vertexCount, 1.0);

  // In order to make the problem well posed, we are going to introduce
  // a linear constraint that the average value of the optimized
  // performances should be something fixed, say 0.5. If the optimized
  // performances are P, then we can express P(Q)=AQ + B with A and B being
  // constant and Q being one dimension less than P. When P is evaluated
  // for any real-valued vector Q, the constraint will be respected. So
  // we are in fact going to optimize an unconstrained problem w.r.t. Q.
  SumConstraint perfSumCst = SumConstraint::averageConstraint(pts.size(), 0.5);

  // This corresponds to the Q vector in the previous explanation.
  Array<double> perfCoeffs = Array<double>::fill(
      perfSumCst.coeffCount(), 1.0);

  ceres::Problem problem;

  // There are two classes of unknowns:
  // (i) The vertices of the surface that we are
  // optimizing
  for (auto& v: vertices) {
    problem.AddParameterBlock(&v, 1);
  }
  // and (ii) the performance at every point.
  for (auto& p: perfCoeffs) {
    problem.AddParameterBlock(&p, 1);
  }
  // Not that the solution will have to be
  // scaled in a post processing step.

  // The data terms correspond to minimizing
  // (S_i*p_i - B_i)^2 for every sample i, with
  // S_i being the fitted target speed surface at sample i,
  // p_i being our estimated performance at that point, and
  // B_i being the observed point speed.
  int goodCount = 0;
  for (int i = 0; i < pointCount; i++) {
    auto fit = makePerfFitPoint(pts, i, settings);
    if (fit.good) {
      auto vp = getVertexPointers(fit.weights, vertices);

      goodCount++;

      // Determine the linear combination
      // of the performance coefficients that
      // is used to compute the performance for this sample.
      // Generally, it is a linear combination of two
      // performance coefficients, and a constant offset.
      auto c = perfSumCst.get(i);

      auto cost = DataFitCost::make(fit, c);

      // Get pointers to the performance coefficients.
      auto perfPtrs = c.pointers(perfCoeffs.getData());

      problem.AddResidualBlock(
          cost,
          settings.dataLoss, // Squared residuals
          vp[0], vp[1], vp[2], // Vertex pointers
          perfPtrs.first,
          perfPtrs.second); // Estimated performance
    }
  }

  LOG(INFO) << "Number of good points: " << goodCount << std::endl;

  // Assume that the estimated performances vary smoothly in time,
  // and penalize differences.
  int perfPairCount = pts.size()-1;
  LOG(INFO) << "Using reg weight " << settings.perfRegWeight << std::endl;
  for (int i = 0; i < perfPairCount; i++) {
    if (pts[i].groupIndex == pts[i+1].groupIndex) {
      auto a = perfSumCst.get(i);
      auto b = perfSumCst.get(i+1);

      auto c = combDiff(a, b);

      auto cp = getPointers(c, perfCoeffs);

      problem.AddResidualBlock(
          ParamRegCost::make(settings.perfRegWeight, c),
          nullptr,
          cp[0], cp[1], cp[2]);
    }
  }

  // Regularize the target speed surface so that it is smooth.
  auto surfaceWeight = settings.vertexRegWeight;
  LOG(INFO) << "Using surface weight " << surfaceWeight << std::endl;
  for (const auto& pair: surfaceNeighbors) {
    problem.AddResidualBlock(
        RegCost::make(surfaceWeight),
        nullptr,
        &(vertices[pair.first]), &(vertices[pair.second]));
  }

  ceres::Solver::Options options;
  ceres::Solver::Summary summary;

  ceres::Solve(options, &problem, &summary);
  LOG(INFO) << "Information about the PerfSurf optimization, after finished:";
  LOG(INFO) << summary.FullReport();

  return RawPerfSurfResults{
    vertices, perfSumCst.apply(perfCoeffs)
  };
}

// The actual implementation of the
// heuristic to find a good scaling factor.
double binarySearchPerfThresholdSub(
    int left, double factor,
    const Array<double>& part) {
  LOG(INFO) << "At " << left << " with perf="
      << part[0] << " and expected-perf=" << factor*left;
  int n = part.size();
  if (n == 1) {
    return part[0];
  } else {
    int middle = n/2;
    int right = left + middle;
    return part[middle] < factor*right?
        binarySearchPerfThresholdSub(right, factor, part.sliceFrom(middle))
      : binarySearchPerfThresholdSub(left, factor, part.sliceTo(middle));
  }
}

// Heuristic, used when determining the
// scaling of the final result.
double binarySearchPerfThreshold(
    const Array<double>& sortedRawPerfs,
    double startQuantile,
    double marg) {
  CHECK(std::is_sorted(sortedRawPerfs.begin(), sortedRawPerfs.end()));
  int n = sortedRawPerfs.size();
  if (n == 1) {
    return sortedRawPerfs.first();
  }
  CHECK(0 < n);
  int index = int(round(n*startQuantile));
  auto part = sortedRawPerfs.sliceFrom(index);
  double factor = (1.0 + marg)*(sortedRawPerfs[index]/index);
  return binarySearchPerfThresholdSub(
      index, factor, part);
}

double estimateSolutionScaling(
    const RawPerfSurfResults& src,
    const PerfSurfSettings& settings) {
  // We are going to sort all the raw performances
  // for the sake of picking one at a quantile
  // in a way that ignores outliers.
  auto perfs = src.rawPerformances.dup();
  if (perfs.empty()) {
    return -1;
  }
  std::sort(perfs.begin(), perfs.end());

  // Small heuristic for finding a good quantile.
  // Start at settings.surfaceQuantile, then try
  // to push up that quantile until we are touching
  // outliers.
  return binarySearchPerfThreshold(
      perfs,
      settings.surfaceQuantile,
      settings.quantileMarg);
}

PerfSurfResults postprocessResults(
    const RawPerfSurfResults& src,
    const PerfSurfSettings& settings) {

  auto factor = estimateSolutionScaling(src, settings);
  if (factor < 0) {
    return PerfSurfResults();
  }

  auto normalizedVertices = src.rawNormalizedVertices.dup();

  int n = normalizedVertices.size();
  auto vertices = Array<Velocity<double>>(n);

  // For every normalized target speed surface vertex, scale it
  // using the reference perf previously found. This is as if the reference
  // perf would be 1.0, then that point would intersect the surface.
  // That is how we scale the surface.
  //
  // Then we just reconstruct the true vertices by multiplying the
  // normalized vertices with the reference speed functino.
  for (int i = 0; i < n; i++) {
    normalizedVertices[i] *= factor;
    vertices[i] = normalizedVertices[i]*settings.refSpeed({
      WeightedIndex(i, 1.0)
    });
  }

  auto scaledPerformances = src.rawPerformances.dup();

  for (auto& p: scaledPerformances) {
    p *= factor;
  }

  return PerfSurfResults{
    scaledPerformances,
    normalizedVertices,
    vertices
  };
}

PerfSurfResults optimizePerfSurf(
    const Array<PerfSurfPt>& pts,
    const Array<std::pair<int, int>>& surfaceNeighbors,
    const PerfSurfSettings& settings) {
  auto subResults = optimizePerfSurfSub(
      pts, surfaceNeighbors, settings);
  return postprocessResults(subResults, settings);
}

} /* namespace sail */
