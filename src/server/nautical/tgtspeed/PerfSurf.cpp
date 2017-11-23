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
    const Array<PerfSurfPt>& data, int index,
    const Eigen::VectorXd& level, const PerfSurfSettings& s) {
  const auto& x = data[index];
  PerfFitPoint pt;
  pt.index = index;
  pt.normedSpeed = x.boatSpeed/s.refSpeed(x);
  pt.weights = scaleWeights(pt.normedSpeed, x.windVertexWeights);
  pt.level = evaluateLevel(pt.weights, level);
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

void outputWeights(
    int row, double s,
    const Array<WeightedIndex>& weights,
    std::vector<Eigen::Triplet<double>>* dst) {
  for (auto w: weights) {
    dst->push_back(Eigen::Triplet<double>(row, w.index, w.weight*s));
  }
}

void generateTriplets(
    const PerfFitPair& p,
    int row,
    std::vector<Eigen::Triplet<double>>* dst) {
  outputWeights(row, 1, p.a.weights, dst);
  outputWeights(row, -1, p.b.weights, dst);
}

Eigen::SparseMatrix<double> makeDataMatrix(
    int n,
    const Array<PerfFitPair>& pairs) {
  std::vector<Eigen::Triplet<double>> triplets;
  for (int i = 0; i < pairs.size(); i++) {
    const auto& p = pairs[i];
    generateTriplets(p, i, &triplets);
  }
  Eigen::SparseMatrix<double> A(pairs.size(), n);
  A.setFromTriplets(triplets.begin(), triplets.end());
  return A;
}

Eigen::VectorXd iterateLevels(
    Eigen::MatrixXd* AtA,
    const Array<PerfFitPair>& pairs,
    const Eigen::MatrixXd& R,
    const PerfSurfSettings& s) {
  int n = AtA->rows();
  auto dataMat = makeDataMatrix(n, pairs);
  Eigen::MatrixXd DtD = (dataMat.transpose()*dataMat).toDense();
  (*AtA) += ((s.regPerCorr*pairs.size() + s.constantReg)*R + DtD);
  return solveConstrained(*AtA, s.type);
}

LevelResults optimizeLevels(
    const Array<PerfSurfPt>& data,
    const Array<std::pair<int, int>>& pairs,
    const Eigen::MatrixXd& reg,
    const PerfSurfSettings& settings) {

  int vertex_count = reg.cols();
  Eigen::MatrixXd R = reg.transpose()*reg;
  Eigen::MatrixXd AtA = Eigen::MatrixXd::Zero(vertex_count, vertex_count);
  Eigen::VectorXd X = Eigen::VectorXd::Ones(vertex_count);

  ArrayBuilder<Array<double>> levels;
  Array<PerfFitPair> goodPairs;
  for (int i = 0; i < settings.iterations; i++) {
    goodPairs = identifyGoodPairs(data, pairs, X, settings);
    LOG(INFO) << "Iteration " << i << " median diff: "
        << goodPairs[goodPairs.size()/2].diff;
    LOG(INFO) << "Max diff: " << goodPairs.last().diff;
    X = iterateLevels(&AtA, goodPairs, R, settings);
    levels.add(Array<double>(vertex_count, X.data()).dup());
    LOG(INFO) << "X = " << X.transpose();
  }
  LevelResults r;
  r.levels = levels.get();
  r.finalPairs = goodPairs;
  return r;
}

} /* namespace sail */
