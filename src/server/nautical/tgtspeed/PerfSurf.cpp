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



Eigen::VectorXd solveLsq(
    const Eigen::SparseMatrix<double>& mat,
    std::vector<double>& rhs) {
  CHECK(mat.rows() == rhs.size());
  Eigen::SparseMatrix<double> AtA = mat.transpose()*mat;
  Eigen::VectorXd AtB = mat.transpose()*Eigen::Map<Eigen::VectorXd>(
      rhs.data(), rhs.size());

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> decomp(AtA);
  Eigen::VectorXd nextVertices = decomp.solve(AtB);
  CHECK(nextVertices.size() == AtB.size());
  return nextVertices;
}

Array<std::pair<int, int>> generatePairs(const Array<Spani>& spans, int step) {
  ArrayBuilder<std::pair<int, int>> dst;
  for (auto span: spans) {
    for (auto i = span.minv(); i + step < span.maxv(); i++) {
      dst.add(std::pair<int, int>(i, i + step));
    }
  }
  return dst.get();
}

Eigen::MatrixXd makeOneDimensionalReg(int n, int order) {
  if (order <= 0) {
    return Eigen::MatrixXd::Identity(n, n);
  } else {
    auto A = makeOneDimensionalReg(n, order-1);
    int m = A.rows();
    return A.block(0, 0, m-1, n) - A.block(1, 0, m-1, n);
  }
}

Eigen::MatrixXd makeC(int n) {
  Eigen::MatrixXd C = Eigen::MatrixXd::Zero(n, n-1);
  for (int i = 0; i < n-1; i++) {
    C(i, i) = 1.0;
    C(i+1, i) = -1.0;
  }
  return C;
}

Eigen::VectorXd solveConstrained(
    const Eigen::MatrixXd& AtA,
    SystemConstraintType type) {
  if (type == SystemConstraintType::Norm1) {
    // Select the minimum eigen vector
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> decomp(AtA);
    return decomp.eigenvectors().col(0);
  } else {

    // Solve AY = 0 with
    // Y = CX + D
    // C and D are chose so that the sum of Y is 1 for any real vector X.

    CHECK(AtA.rows() == AtA.cols());
    int n = AtA.rows();
    Eigen::MatrixXd C = makeC(n);
    Eigen::VectorXd D = (1.0/n)*Eigen::VectorXd::Ones(n);
    Eigen::MatrixXd CtAtA = C.transpose()*AtA;
    Eigen::MatrixXd CtAtAC = CtAtA*C;
    Eigen::MatrixXd minusCtAtAD = -CtAtA*D;
    CHECK(CtAtAC.rows() == n-1);
    CHECK(CtAtAC.cols() == n-1);
    CHECK(minusCtAtAD.rows() == n-1);
    CHECK(minusCtAtAD.cols() == 1);
    return C*CtAtAC.ldlt().solve(minusCtAtAD) + D;
  }
}

double evaluateLevel(
    const Array<WeightedIndex>& weights,
    const Eigen::VectorXd& X) {
  double s = 0.0;
  for (auto w: weights) {
    s += X(w.index)*w.weight;
  }
  return s;
}

PerfFitPoint makePerfFitPoint(
    const Array<PerfSurfPt>& data, int index,
    const Eigen::VectorXd& level, const PerfSurfSettings& s) {
  const auto& x = data[index];
  PerfFitPoint pt;
  pt.index = index;
  pt.level = evaluateLevel(x.windVertexWeights, level);
  pt.normedSpeed = x.boatSpeed/s.refSpeed(x);
  pt.weights = x.windVertexWeights;
  pt.good = std::isfinite(pt.normedSpeed);
  return pt;
}



Array<PerfFitPair> identifyGoodPairs(
    const Array<PerfSurfPt>& data,
    const Array<std::pair<int, int>>& pairs,
    const Eigen::VectorXd& X,
    const PerfSurfSettings& s) {
  int n = pairs.size();
  ArrayBuilder<PerfFitPair> candidates(n);
  {
    for (int i = 0; i < n; i++) {
      auto p = pairs[i];
      PerfFitPair c;
      c.a = makePerfFitPoint(data, p.first, X, s);
      c.b = makePerfFitPoint(data, p.second, X, s);
      if (c.a.good && c.b.good) {
        c.diff = std::abs(c.a.level - c.b.level);
        candidates.add(c);
      }
    }
  }
  auto cands = candidates.get();
  std::sort(cands.begin(), cands.end());
  return cands.sliceTo(int(round(cands.size()*s.goodFraction)));
}

Array<Array<double>> optimizeLevels(
    const Array<PerfSurfPt>& data,
    const Array<std::pair<int, int>>& pairs,
    const Eigen::MatrixXd& reg,
    const PerfSurfSettings& settings) {

  int vertex_count = reg.cols();
  Eigen::MatrixXd AtA = Eigen::MatrixXd::Zero(vertex_count, vertex_count);
  Eigen::VectorXd X = Eigen::VectorXd::Ones(vertex_count);

  ArrayBuilder<Array<double>> results;
  for (int i = 0; i < settings.iterations; i++) {
    auto goodPairs = identifyGoodPairs(data, pairs, X, settings);
    results.add(Array<double>(vertex_count, X.data()).dup());
  }
  return results.get();
}

} /* namespace sail */
