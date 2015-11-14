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

Array<Arrayi> makeRandomSplit(int sampleCount0, int splitCount, RandomEngine *rng) {
  int samplesPerSplit = sampleCount0/splitCount;
  int n = splitCount*samplesPerSplit;
  Arrayi inds(n);
  LineKM map(0, n, 0, splitCount);
  for (int i = 0; i < n; i++) {
    inds[i] = int(floor(map(i)));
  }

  if (rng == nullptr) {
    std::random_shuffle(inds.begin(), inds.end());
  } else {
    auto gen = [&](int index) {
      return std::uniform_int_distribution<int>(0, index)(*rng);
    };
    std::random_shuffle(inds.begin(), inds.end(), gen);
  }

  Array<ArrayBuilder<int> > splits(splitCount);
  for (int i = 0; i < n; i++) {
    splits[inds[i]].add(i);
  }
  return splits.map<Arrayi>([=](ArrayBuilder<int> b) {
    return b.get();
  });
}

Array<Spani> makeContiguousSpans(int sampleCount, int splitSize) {
  int splitCount = sampleCount/splitSize;
  return Spani(0, splitCount).map<Spani>([&](int index) {
    int from = index*splitSize;
    int to = from + splitSize;
    return Spani(from, to);
  });
}



// Inside the optimizer, where T is a ceres Jet.
// Not sure how well it works with Eigen.
template <typename T>
void matMulAdd(T scale, const Eigen::MatrixXd &A,
               const Eigen::VectorXd &B, const T *X, T *dst) {
  CHECK(A.rows() == B.rows());
  for (int i = 0; i < A.rows(); i++) {
    T sum = T(B(i));
    for (int j = 0; j < A.cols(); j++) {
      sum += A(i, j)*X[j];
    }
    T result = scale*sum;
    dst[i] = result;
  }
}

template <typename T>
T matMulSquaredNorm(const Eigen::MatrixXd &A, const T *X) {
  T sumSquares = T(0.0);
  for (int i = 0; i < A.rows(); i++) {
    T sum = T(0.0);
    for (int j = 0; j < A.cols(); j++) {
      sum += A(i, j)*X[j];
    }
    sumSquares += sum*sum;
  }
  return sumSquares;
}


Arrayd initializeParameters(const Arrayd &Xprovided, int count) {
  if (Xprovided.empty()) {
    auto X = Arrayd::fill(count, 0.0);
    X[0] = 1.0;
    return X;
  }
  CHECK(Xprovided.size() == count);
  return Xprovided;
}


auto getRowBlock(Eigen::MatrixXd &X, int index, int blockSize)
  -> decltype(X.block(0, 0, 1, 1)) {
  return X.block(index*blockSize, 0, blockSize, X.cols());
}

int getBlockCount(int rows, int blockSize) {
  int n = rows/blockSize;
  CHECK(blockSize*n == rows);
  return n;
}

Eigen::MatrixXd computeMean(Eigen::MatrixXd &X, int dim) {
  int n = getBlockCount(X.rows(), dim);
  Eigen::MatrixXd sum = Eigen::MatrixXd::Zero(dim, X.cols());
  for (int i = 0; i < n; i++) {
    sum += getRowBlock(X, i, dim);
  }
  return (1.0/n)*sum;
}


Eigen::MatrixXd subtractMean(Eigen::MatrixXd A, int dim) {
  auto mean = computeMean(A, dim);
  int n = getBlockCount(A.rows(), dim);
  Eigen::MatrixXd dst(A.rows(), A.cols());
  for (int i = 0; i < n; i++) {
    getRowBlock(dst, i, dim) = getRowBlock(A, i, dim) - mean;
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

Eigen::MatrixXd integrateFlowData(Eigen::MatrixXd X) {
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

FlowFiber operator+(const FlowFiber &a, const FlowFiber &b) {
  CHECK(a.sameSizeAs(b));
  return FlowFiber{a.Q + b.Q, a.B + b.B};
}

void makeConstantFlowTrajectoryMatrix(DataFit::CoordIndexer rows,
                                          DataFit::CoordIndexer cols,
                                          std::vector<DataFit::Triplet> *dst) {
  int dim = rows.dim();
  CHECK(rows.dim() == cols.dim());
  CHECK(cols.count() == 2);
  auto variableColSpan = cols.span(0);
  auto constantColSpan = cols.span(1);
  for (int i = 0; i < rows.count(); i++) {
    auto rowSpan = rows.span(i);
    for (int j = 0; j < dim; j++) {
      using namespace DataFit;
      int row = rowSpan[j];
      dst->push_back(Triplet(row, variableColSpan[j], i));
      dst->push_back(Triplet(row, constantColSpan[j], 1.0));
    }
  }
}


FlowFiber operator-(const FlowFiber &a, const FlowFiber &b) {
  CHECK(a.sameSizeAs(b));
  return FlowFiber{a.Q - b.Q, a.B - b.B};
}

FlowFiber operator-(const FlowFiber &a) {
  return (-1.0)*a;
}

FlowFiber operator*(double f, const FlowFiber &b) {
  return FlowFiber{f*b.Q, f*b.B};
}


double FlowFiber::eval(Eigen::VectorXd params, double scale) {
  Eigen::VectorXd result = Q*params + scale*B;
  return result.norm();
}

FlowFiber FlowFiber::differentiate() const {
  int cols = Q.cols();
  int dstRows = Q.rows() - 2;
  auto Q0 = Q.block(0, 0, dstRows, cols);
  auto Q1 = Q.block(2, 0, dstRows, cols);
  auto B0 = B.block(0, 0, dstRows, 1);
  auto B1 = B.block(2, 0, dstRows, 1);
  return FlowFiber{Q1 - Q0, B1 - B0};
}

FlowFiber FlowFiber::integrate() const {
  return FlowFiber{integrateFlowData(Q), integrateFlowData(B)};
}

FlowFiber FlowFiber::dropConstant() const {
  return FlowFiber{Q, Eigen::MatrixXd::Zero(B.rows(), B.cols())};
}

FlowFiber FlowFiber::dropVariable() const {
  return FlowFiber{Eigen::MatrixXd::Zero(Q.rows(), Q.cols()), B};
}

bool FlowFiber::sameSizeAs(const FlowFiber &other) const {
  return rows() == other.rows() && parameterCount() == other.parameterCount();
}

Eigen::VectorXd FlowFiber::minimizeNorm() const {
  return Q.colPivHouseholderQr().solve(B);
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

void plotFlowFibers(Array<FlowFiber> data,
    Eigen::VectorXd A, Eigen::VectorXd B, double scale) {
  GnuplotExtra plot;
  plot.set_style("lines");
  plot.cmd("set style line 1 lt rgb \"blue\" lw 1 pt 6");
  plot.cmd("set style line 2 lt rgb \"green\" lw 1 pt 6");
  for (auto d: data) {
    plot.set_current_line_style("1");
    plot.plot(d.makePlotData(A, scale));
    plot.set_current_line_style("2");
    plot.plot(d.makePlotData(B, scale));
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
    auto q = integrateFlowData(extractRows(Q, split, 2));
    auto b = integrateFlowData(extractRows(B, split, 2));
    return FlowFiber{q, b};
  });
}

Array<FlowFiber> computeFiberMeans(Array<FlowFiber> rawFibers, int dstCount) {
  Array<FlowFiber> dst(dstCount);

  LineKM m(0, dstCount, 0, rawFibers.size());
  for (int i = 0; i < dstCount; i++) {
    int from = int(floor(m(i)));
    int to = int(floor(m(i + 1)));
    dst[i] = computeMeanFiber(rawFibers.slice(from, to));
  }
  return dst;
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


FlowFiber computeMeanFiber(Array<FlowFiber> fibers) {
  auto fiber0 = fibers[0].Q;
  Eigen::MatrixXd Q(fiber0.rows(), fiber0.cols());
  Eigen::VectorXd B(fiber0.rows());
  for (auto f: fibers) {
    Q += f.Q;
    B += f.B;
  }
  double f = 1.0/fibers.size();
  return FlowFiber{f*Q, f*B};
}

FlowFiber buildFitnessFiber(Array<FlowFiber> fibers, FlowFiber dst) {
  int cols = getSameCount(fibers, [](const FlowFiber &f) {return f.parameterCount();});
  int rowsPerFiber = getSameCount(fibers, [](const FlowFiber &f) {return f.rows();});

  auto n = fibers.size();
  Eigen::MatrixXd Q(n*rowsPerFiber, cols);
  Eigen::MatrixXd B(n*rowsPerFiber, 1);
  for (int i = 0; i < n; i++) {
    auto &fiber = fibers[i];
    getRowBlock(Q, i, rowsPerFiber) = fiber.Q - dst.Q;
    getRowBlock(B, i, rowsPerFiber) = fiber.B - dst.B;
  }
  return FlowFiber{Q, B};
}



/*Array<NormedData> assembleNormedData(FlowMatrices mats, Array<Arrayi> splits) {

}*/



}
}
