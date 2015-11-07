/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/calibration/LinearCalibration.h>
#include <armadillo>
#include <server/math/BandMat.h>
#include <server/common/ArrayIO.h>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/math/nonlinear/DataFit.h>
#include <server/plot/extra.h>
#include <server/common/ArrayBuilder.h>
#include <ceres/ceres.h>

namespace sail {
namespace LinearCalibration {

void initializeParameters(bool withOffset, double *dst) {
  int n = flowParamCount(withOffset);
  for (int i = 0; i < n; i++) {
    dst[i] = 0.0;
  }
  dst[0] = 1.0;
}

int calcPassiveCount(Duration<double> total, Duration<double> period) {
  return 1 + int(floor(total/period));
}

LinearCorrector::LinearCorrector(const FlowSettings &flowSettings,
    Arrayd windParams, Arrayd currentParams) :
    _flowSettings(flowSettings), _windParams(windParams), _currentParams(currentParams) {}

Array<CalibratedNav<double> > LinearCorrector::operator()(const Array<Nav> &navs) const {
  return navs.map<CalibratedNav<double> >([&](const Nav &x) {
    return (*this)(x);
  });
}

arma::mat asMatrix(const MDArray2d &x) {
  return arma::mat(x.ptr(), x.rows(), x.cols(), false, true);
}

arma::mat asMatrix(const Arrayd &x) {
  return arma::mat(x.ptr(), x.size(), 1, false, true);
}

CalibratedNav<double> LinearCorrector::operator()(const Nav &nav) const {
  MDArray2d Aw(2, _flowSettings.windParamCount()), Bw(2, 1);
  MDArray2d Ac(2, _flowSettings.currentParamCount()), Bc(2, 1);

  makeTrueWindMatrixExpression(nav, _flowSettings, &Aw, &Bw);
  makeTrueCurrentMatrixExpression(nav, _flowSettings, &Ac, &Bc);
  arma::vec2 windMat = asMatrix(Aw)*asMatrix(_windParams) + asMatrix(Bw);
  arma::vec2 currentMat = asMatrix(Ac)*asMatrix(_currentParams) + asMatrix(Bc);

  HorizontalMotion<double> wind{Velocity<double>::knots(windMat[0]),
    Velocity<double>::knots(windMat[1])};
  HorizontalMotion<double> current{Velocity<double>::knots(currentMat[0]),
    Velocity<double>::knots(currentMat[1])};
  CalibratedNav<double> dst;
  dst.rawAwa.set(nav.awa());
  dst.rawAws.set(nav.aws());
  dst.rawMagHdg.set(nav.magHdg());
  dst.rawWatSpeed.set(nav.watSpeed());
  dst.gpsMotion.set(nav.gpsMotion());

  // TODO: Fill in more things here.

  dst.trueWindOverGround.set(wind);
  dst.trueCurrentOverGround.set(current);
  return dst;
}

std::string LinearCorrector::toString() const {
  std::stringstream ss;
  ss << "LinearCorrector(windParams="<< _windParams << ", currentParams=" << _currentParams << ")";
  return ss.str();
}

Array<Arrayi> makeRandomSplit(int sampleCount0, int splitCount) {
  int samplesPerSplit = sampleCount0/splitCount;
  int n = splitCount*samplesPerSplit;
  Arrayi inds(n);
  LineKM map(0, n, 0, splitCount);
  for (int i = 0; i < n; i++) {
    inds[i] = int(floor(map(i)));
  }
  std::random_shuffle(inds.begin(), inds.end());
  Array<ArrayBuilder<int> > splits(splitCount);
  for (int i = 0; i < n; i++) {
    splits[inds[i]].add(i);
  }
  return splits.map<Arrayi>([=](ArrayBuilder<int> b) {
    return b.get();
  });
}

Eigen::MatrixXd subtractMean(Eigen::MatrixXd A, int dim) {
  int rows = A.rows();
  int cols = A.cols();
  Eigen::MatrixXd dst = Eigen::MatrixXd(rows, cols);
  assert(rows % dim == 0);

  int count = rows/dim;
  assert(dim*count == rows);
  Eigen::MatrixXd sum = Eigen::MatrixXd::Zero(dim, cols);
  {
    int offset = 0;
    for (int i = 0; i < count; i++) {
      sum += A.block(offset, 0, dim, cols);
      offset += dim;
    }
  }
  sum *= (1.0/count);
  {
    int offset = 0;
    for (int i = 0; i < count; i++) {
      dst.block(offset, 0, dim, cols) = A.block(offset, 0, dim, cols) - sum;
      offset += dim;
    }
  }
  return dst;
}

Eigen::MatrixXd integrate(Eigen::MatrixXd A, int dim) {
  int rows = A.rows();
  int cols = A.cols();
  int count = rows/dim;
  Eigen::MatrixXd sum = Eigen::MatrixXd::Zero(dim, cols);
  Eigen::MatrixXd B((1 + count)*dim, cols);
  B.block(0, 0, dim, cols) = sum;
  for (int i = 0; i < count; i++) {
    int srcOffset = i*dim;
    int dstOffset = srcOffset + dim;
    auto a = A.block(srcOffset, 0, dim, cols);
    sum += a;
    B.block(dstOffset, 0, dim, cols) = sum;
  }
  return B;
}

Eigen::MatrixXd normalizeFlowData(Eigen::MatrixXd X) {
  return subtractMean(integrate(X, 2), 2);
}

MDArray2d FlowFiber::makePlotData(Eigen::VectorXd params, double scale) {
  Eigen::VectorXd Y = Q*params + scale*B;
  int n = Y.rows()/2;
  MDArray2d dst(n, 2);
  for (int i = 0; i < n; i++) {
    int offset = 2*i;
    dst(i, 0) = Y[offset + 0];
    dst(i, 1) = Y[offset + 1];
  }
  return dst;
}

double FlowFiber::eval(Eigen::VectorXd params, double scale) {
  Eigen::VectorXd result = Q*params + scale*B;
  return result.norm();
}

void plotFlowFibers(Array<FlowFiber> data,
                      Eigen::VectorXd params, double scale) {
  GnuplotExtra plot;
  plot.set_style("lines");
  for (auto d: data) {
    plot.plot(d.makePlotData(params, scale));
  }
  plot.show();
}

Eigen::MatrixXd extractRows(Eigen::MatrixXd mat, Arrayi inds, int dim) {
  int n = inds.size();
  int dstRows = dim*n;
  int cols = mat.cols();
  Eigen::MatrixXd dst(dstRows, cols);
  for (int i = 0; i < n; i++) {
    int dstRowOffset = i*dim;
    int srcRowOffset = inds[i]*dim;
    dst.block(dstRowOffset, 0, dim, cols) = mat.block(srcRowOffset, 0, dim, cols);
  }
  return dst;
}

Array<FlowFiber> makeFlowFibers(Eigen::MatrixXd Q, Eigen::MatrixXd B,
    Array<Arrayi> splits) {
  return splits.map<FlowFiber>([=](Arrayi split) {
    auto q = normalizeFlowData(extractRows(Q, split, 2));
    auto b = normalizeFlowData(extractRows(B, split, 2));
    return FlowFiber{q, b};
  });
}

Array<Eigen::MatrixXd> makeFlowFibers(Eigen::MatrixXd Q, Array<Arrayi> splits) {
  return splits.map<Eigen::MatrixXd>([=](Arrayi split) {
    return Eigen::MatrixXd(normalizeFlowData(extractRows(Q, split, 2)));
  });
}

int getSameCount(Array<FlowFiber> fibers, std::function<int(FlowFiber)> f) {
  if (fibers.empty()) {
    return -1;
  } else {
    auto n = f(fibers[0]);
    for (auto fiber: fibers) {
      if (n != f(fiber)) {
        return -1;
      }
    }
    return n;
  }
}


FlowFiber assembleFullProblem(Array<FlowFiber> fibers) {
  using namespace DataFit;
  auto rowsPerFiber = getSameCount(fibers, [](FlowFiber f) {return f.rows();});
  auto parametersPerFiber = getSameCount(fibers, [](FlowFiber f) {
    return f.parameterCount();
  });
  CHECK(rowsPerFiber != -1);
  CHECK(parametersPerFiber != -1);
  int fiberCount = fibers.size();
  int rows = fiberCount*rowsPerFiber;
  Eigen::MatrixXd Q(rows, parametersPerFiber);
  Eigen::MatrixXd B(rows, 1);
  auto idx = CoordIndexer::Factory().make(fiberCount, rowsPerFiber);
  for (int i = 0; i < fiberCount; i++) {
    auto s = idx.span(i);
    sliceRows(Q, s) = sliceRows(fibers[i].Q, s);
    sliceRows(B, s) = sliceRows(fibers[i].B, s);
  }
  return FlowFiber{
    subtractMean(Q, rowsPerFiber),
    subtractMean(B, rowsPerFiber)
  };
}

Eigen::VectorXd smallestEigVec(const Eigen::MatrixXd &K) {
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(K);
  double minValue = std::numeric_limits<double>::infinity();
  int best = -1;
  for (int i = 0; i < solver.eigenvalues().size(); i++) {
    auto val = solver.eigenvalues()[i];
    if (val < minValue) {
      best = i;
      minValue = val;
    }
  }
  return solver.eigenvectors().block(0, best, K.rows(), 1);
}

Eigen::MatrixXd assembleFullProblem(Array<Eigen::MatrixXd> fibers) {
  using namespace DataFit;
  int rowsPerFiber = fibers[0].rows();
  int cols = fibers[0].cols();
  int fiberCount = fibers.size();
  auto idx = CoordIndexer::Factory().make(fiberCount, rowsPerFiber);
  int rows = fiberCount*rowsPerFiber;
  Eigen::MatrixXd Q(rows, cols);
  for (int i = 0; i < fiberCount; i++) {
    sliceRows(Q, idx.span(i)) = fibers[i];
  }
  return subtractMean(Q, rowsPerFiber);
}

Eigen::MatrixXd makeAB(const Eigen::MatrixXd &A, const Eigen::MatrixXd &B) {
  CHECK(A.rows() == B.rows());
  int rows = A.rows();
  CHECK(B.cols() == 1);
  Eigen::MatrixXd AB(rows, A.cols() + 1);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < A.cols(); j++) {
      AB(i, j) = A(i, j);
      AB(i, A.cols()) = B(i, 0);
    }
  }
  return AB;
}


namespace {

  class Objf {
   public:
    Objf(FlowFiber fibers, Eigen::MatrixXd A, Eigen::MatrixXd B) :
      _fibers(fibers), _A(A), _B(B) {}

    int outDims() const {
      return _fibers.rows();
    }

    int inDims() const {
      return _fibers.parameterCount();
    }

    template<typename T>
    bool operator()(T const* const* parameters, T* residuals) const {
      const T *X = parameters[0];
      return eval(X, residuals);
    }
   private:
    FlowFiber _fibers;
    Eigen::MatrixXd _A, _B;

    template <typename T>
    bool eval(const T *X,
        T *residuals) const {
      T len = computeTotalTrajectoryLength(X);
    }

    template <typename T>
    T matmulAXB(int rowIndex, const Eigen::MatrixXd &A,
        const Eigen::MatrixXd &B, const T *X) const {
      T sum = T(B.rows() > 0? B(rowIndex, 0) : 0.0);
      for (int i = 0; i < A.cols(); i++) {
        sum += A(rowIndex, i)*X[i];
      }
      return sum;
    }

    template <typename T>
    T computeTotalTrajectoryLength(const T *X) const {
      T totalLength(0.0);
      int n = _fibers.observationCount();
      for (int i = 0; i < n; i++) {
        int offset = 2*i;
        auto x = matmulAXB(offset + 0, _A, _B, X);
        auto y = matmulAXB(offset + 1, _A, _B, X);
        totalLength += sqrt(x*x + y*y);
      }
      return totalLength;
    }
  };

}


NLResults optimizeNonlinear(FlowMatrices flow, Array<Arrayi> splits) {
  Eigen::MatrixXd Aeigen =
      Eigen::Map<Eigen::MatrixXd>(flow.A.ptr(), flow.rows(), flow.A.cols());

  Eigen::MatrixXd Beigen =
      Eigen::Map<Eigen::MatrixXd>(flow.B.ptr(), flow.rows(), 1);
  auto fibers = makeFlowFibers(Aeigen, Beigen, splits);
  auto full = assembleFullProblem(fibers);

  Arrayd parameters = Arrayd::fill(full.parameterCount(), 0.0);
  parameters[0] = 1.0;

  auto objf = new Objf(full, Aeigen, Beigen);
  ceres::Problem problem;
  auto cost = new ceres::DynamicAutoDiffCostFunction<Objf>(objf);
  cost->AddParameterBlock(full.parameterCount());
  cost->SetNumResiduals(objf->outDims());
  problem.AddResidualBlock(cost, NULL, parameters.ptr());
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  options.max_num_iterations = 60;
  ceres::Solver::Summary summary;
  Solve(options, &problem, &summary);
  return NLResults{
    Aeigen, Beigen, fibers, parameters
  };
}

/*Array<NormedData> assembleNormedData(FlowMatrices mats, Array<Arrayi> splits) {

}*/



}
}
