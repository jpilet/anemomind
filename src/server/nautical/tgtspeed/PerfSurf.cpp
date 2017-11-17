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

Velocity<double> evaluateSurfaceSpeed(
    const Array<WeightedIndex>& weights,
    const Array<Velocity<double>>& vertices) {
  Velocity<double> v = 0.0_kn;
  for (const auto& w: weights) {
    v += w.weight*vertices[w.index];
  }
  return v;
}

Array<PerfSurfPt> updateAndRegularizePerformancePerWindow(
    const Array<PerfSurfPt>& samples,
    const Array<Span<int>>& windows,
    const Array<Velocity<double>>& surfaceVertices,
    const PerfSurfSettings& settings,
    int overlaps) {
  std::vector<double> performances;
  Array<PerfSurfPt> pts(overlaps);
  int n = windows.size();
  ArrayBuilder<Array<PerfSurfPt>> dst(n);
  int at = 0;
  for (int i = 0; i < n; i++) {
    auto w = windows[i];
    int next = at + w.width();

    auto local = pts.slice(at, next);
    samples.slice(w.minv(), w.maxv()).copyToSafe(local);

    performances.resize(w.width()); // Windows are mostly the same size.
    for (int i = 0; i < local.size(); i++) {
      const auto& x = local[i];
      double rawPerf = x.boatSpeed/evaluateSurfaceSpeed(
          x.windVertexWeights, surfaceVertices);
      performances[i] = clamp<double>(
          rawPerf,
          0, 1);
    }
    std::sort(performances.begin(), performances.end());
    double commonPerformance = performances[performances.size()/2];
    for (auto& x: local) {
      x.performance = commonPerformance;
    }
    at = next;
  }
  CHECK(at == pts.size());
  return pts;
}

Array<Velocity<double>> solveSurfaceVerticesLocalOptimizationProblem(
    const Array<PerfSurfPt>& pts,
    const Array<Velocity<double>>& vertices,
    const PerfSurfSettings& settings) {
  std::vector<Eigen::Triplet<double>> triplets;
  Eigen::SparseMatrix<double> mat;
  std::vector<double> rhs;
  rhs.reserve(pts.size());
  mat.setFromTriplets(triplets.begin(), triplets.end());
  for (const auto& pt: pts) {
    int row = rhs.size();
    for (const auto& w : pt.windVertexWeights) {
      triplets.push_back({row, w.index, w.weight*pt.performance});
    }
  }
}

Array<Velocity<double>> iterateSurfaceVertices(
    const Array<PerfSurfPt>& samples,
    const Array<Span<int>>& windows,
    const Array<Velocity<double>>& surfaceVertices,
    const PerfSurfSettings& settings,
    int overlaps) {
  auto updated = updateAndRegularizePerformancePerWindow(
      samples, windows, surfaceVertices, settings, overlaps);
  return solveSurfaceVerticesLocalOptimizationProblem(
      updated,
      surfaceVertices,
      settings);
}

int countOverlaps(const Array<Span<int>>& windows) {
  int n = 0;
  for (auto w: windows) {
    n += w.width();
  }
  return n;
}

Array<Velocity<double>> optimizePerfSurface(
    const Array<PerfSurfPt>& samples,
    const Array<Span<int>>& windows,
    const Array<Velocity<double>>& initialSurfaceVertices,
    const PerfSurfSettings& settings) {
  int overlaps = countOverlaps(windows);
  auto surfaceVertices = initialSurfaceVertices.dup();
  for (int i = 0; i < settings.iterations; i++) {
    surfaceVertices = iterateSurfaceVertices(
        samples, windows, surfaceVertices,
        settings, overlaps);
  }
  return surfaceVertices;
}

} /* namespace sail */
