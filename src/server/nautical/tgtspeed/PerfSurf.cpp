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
#include <server/math/Majorize.h>
#include <ceres/jet.h>

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


ceres::Jet<double, 1> evaluateHuberSub(
    const ceres::Jet<double, 1>& x, double sigma0) {
  if (x < 0.0) {
    return evaluateHuberSub(-x, sigma0);
  } else if (x < sigma0) {
    return x*x;
  } else {
    ceres::Jet<double, 1> sigma(sigma0, 0);
    ceres::Jet<double, 1> s2 = sigma*sigma;
    return s2.a + (x - sigma0)*s2.v[0];
  }
}

ceres::Jet<double, 1> evaluateHuber(double x0, double sigma0) {
  ceres::Jet<double, 1> x(x0, 0);
  return evaluateHuberSub(x, sigma0);
}

Array<Velocity<double>> solveSurfaceVerticesLocalOptimizationProblem(
    const Array<PerfSurfPt>& pts,
    const Array<Velocity<double>>& vertices,
    const PerfSurfSettings& settings) {
  Velocity<double> unit = 1.0_kn;

  std::vector<Eigen::Triplet<double>> triplets;
  Eigen::SparseMatrix<double> mat;
  std::vector<double> rhs;
  rhs.reserve(pts.size());
  std::vector<Eigen::Triplet<double>> localTriplets;
  for (const auto& pt: pts) {
    int row = rhs.size();

    Velocity<double> current = 0.0_kn;
    localTriplets.resize(pt.windVertexWeights.size());
    for (const auto& w : pt.windVertexWeights) {

      // Note: *All* samples contribute to the shape of the surface.
      double weight = w.weight*pt.performance;
      localTriplets.push_back({row, w.index, weight});

      current += weight*vertices[w.index];
    }

    Velocity<double> observed = pt.boatSpeed;

    /*
     * In most cases, we expect the error to be small and due to
     * the regularization. But for the high-performance points and
     * outliers the error might be large.
     */
    Velocity<double> error = current - observed;
    auto h = evaluateHuber(error/unit, settings.sigma/unit);

    LOG(INFO) << "h.a="<< h.a <<"  h.v=" << h.v[0];

    MajQuad maj = MajQuad::majorize(error/unit, h.a, h.v[0])
      + MajQuad::linear(settings.weightPerPoint/unit);
    auto factor = maj.factor();

    LOG(INFO) << "Factor k=" << factor.getK() << " m=" << factor.getM();

    for (auto t: localTriplets) {
      //triplets.push_back({t.row(), t.col(), factor.getK()*t.value()});
    }

    //rhs.push_back(double(observed/unit) - factor.getM());
  }
  mat.setFromTriplets(triplets.begin(), triplets.end());
  //Eigen::SparseMatrix<double> AtA = mat.transpose()*mat;

  return vertices;
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
