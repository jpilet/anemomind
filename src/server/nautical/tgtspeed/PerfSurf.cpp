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

Array<std::pair<int, int>> generateSurfaceNeighbors1d(int vertexCount) {
  int pairCount = vertexCount-1;
  Array<std::pair<int, int>> dst(pairCount);
  for (int i = 0; i < pairCount; i++) {
    dst[i] = {i, i+1};
  }
  return dst;
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
    const Array<PerfSurfPt>& data,
    int index, const PerfSurfSettings& s) {
  const auto& x = data[index];
  PerfFitPoint pt;
  pt.index = index;
  pt.normedSpeed = x.boatSpeed/s.refSpeed(x);
  pt.weights = scaleWeights(pt.normedSpeed, x.windVertexWeights);
  pt.good = std::isfinite(pt.normedSpeed) && pt.normedSpeed < s.maxFactor;
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
  return maxVertex;
}

namespace {
  struct DataFitCost {

    PerfFitPoint pt;

    static ceres::CostFunction* make(
        const PerfFitPoint& p) {
      return new ceres::AutoDiffCostFunction<DataFitCost, 1, 1, 1, 1>(
          new DataFitCost{p});
    }

    template <typename T>
    bool operator()(
        const T* v0, // Vertices
        const T* v1,
        //const T* v2,
        const T* perfPtr,
        T* residual) const {
      typedef const T* Ptr;
      Ptr v[2] = {v0, v1/*, v2*/};
      CHECK(pt.weights.size() <= 3);
      T perf = *perfPtr;
      T interpolatedTargetSpeed = T(0.0);
      for (int i = 0; i < pt.weights.size(); i++) {
        interpolatedTargetSpeed += *(v[i])*pt.weights[i].weight;
      }
      *residual = perf*interpolatedTargetSpeed /*estimated boat speed*/
          - pt.normedSpeed/*observed boat speed*/;

      std::cout << "Residual is " << *residual << std::endl;

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
        const T* a,
        const T* b,
        T* residual) const {
      *residual = weight*(*a - *b);
      return true;
    }
  };

  std::array<double*, 2> getVertexPointers(
      const Array<WeightedIndex>& weights,
      Array<double>& arr) {
    std::array<double*, 2> dst;
    int n = weights.size();
    CHECK(n == 2);
    CHECK(n <= 3);
    for (int i = 0; i < n; i++) {
      dst[i] = &(arr[weights[i].index]);
    }
    //auto last = dst[n-1];

    // Fill the remaining entries with a valid, but unused, value.
    /*for (int i = n; i < dst.size(); i++) {
      dst[i] = nullptr; //last;
    }*/
    return dst;
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

PerfSurfResults optimizePerfSurf(
    const Array<PerfSurfPt>& pts,
    const Array<std::pair<int, int>>& surfaceNeighbors,
    const PerfSurfSettings& settings) {

  int vertexCount = getVertexCount(surfaceNeighbors);

  int pointCount = pts.size();

  // The normalized height of every vertex. The vertices
  // are considered unknown and unconstrained.
  Array<double> vertices = Array<double>::fill(vertexCount, 0.0);

  Array<double> performances = Array<double>::fill(pts.size(), 0.0);

  ceres::Problem problem;
  for (auto& v: vertices) {
    problem.AddParameterBlock(&v, 1);
  }
  for (auto& p: performances) {
    problem.AddParameterBlock(&p, 1);
  }

  // Add the data terms
  int goodCount = 0;
  for (int i = 0; i < pointCount; i++) {
    auto fit = makePerfFitPoint(pts, i, settings);
    if (fit.good) {
      auto vp = getVertexPointers(fit.weights, vertices);
      auto cost = DataFitCost::make(fit);
      goodCount++;
      problem.AddResidualBlock(
          cost,
          nullptr, // Squared residuals
          vp[0], vp[1], //vp[2], // Vertex pointers
          &(performances[i])); // Estimated performance
    }
  }

  LOG(INFO) << "Number of good points: " << goodCount << std::endl;

  // Add regularization for the performance pairs
  // in time
  int perfPairCount = performances.size()-1;
  LOG(INFO) << "Using reg weight " << settings.regWeight << std::endl;
  for (int i = 0; i < perfPairCount; i++) {
    problem.AddResidualBlock(
        RegCost::make(settings.regWeight),
        nullptr,
        &(performances[i]), &(performances[i+1]));
  }

  // Regularize the fitted surface
  auto surfaceWeight = transferWeight(
      perfPairCount, settings.regWeight, vertexCount);
  LOG(INFO) << "Using surface weight " << surfaceWeight << std::endl;
  for (const auto& pair: surfaceNeighbors) {
    problem.AddResidualBlock(
        RegCost::make(surfaceWeight),
        nullptr,
        &(vertices[pair.first]), &(vertices[pair.second]));
  }

  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  std::cout << summary.FullReport() << "\n";

  return PerfSurfResults{
    vertices, performances
  };
}

} /* namespace sail */
