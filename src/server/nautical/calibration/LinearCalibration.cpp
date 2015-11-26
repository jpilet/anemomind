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
#include <server/math/EigenUtils.h>
#include <server/common/Functional.h>

namespace sail {
namespace LinearCalibration {

void initializeParameters(bool withOffset, double *dst) {
  int n = flowParamCount(withOffset);
  for (int i = 0; i < n; i++) {
    dst[i] = 0.0;
  }
  dst[0] = 1.0;
}

Eigen::VectorXd makeXinitEigen() {
  Eigen::VectorXd dst = Eigen::VectorXd::Zero(4);
  initializeParameters(true, dst.data());
  return dst;
}

int calcPassiveCount(Duration<double> total, Duration<double> period) {
  return 1 + int(floor(total/period));
}

LinearCorrector::LinearCorrector(const FlowSettings &flowSettings,
    Arrayd windParams, Arrayd currentParams) :
    _flowSettings(flowSettings), _windParams(windParams), _currentParams(currentParams) {}

Array<CalibratedNav<double> > LinearCorrector::operator()(const Array<Nav> &navs) const {
  return map(navs, [&](const Nav &x) {
    return (*this)(x);
  }).toArray();
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
  LineKM m(0, n, 0, splitCount);
  for (int i = 0; i < n; i++) {
    inds[i] = int(floor(m(i)));
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
  return map(splits, [=](ArrayBuilder<int> b) {
    return b.get();
  }).toArray();
}

Array<Spani> makeContiguousSpans(int sampleCount, int splitSize) {
  int splitCount = sampleCount/splitSize;
  return map(Spani(0, splitCount), [&](int index) {
    int from = index*splitSize;
    int to = from + splitSize;
    return Spani(from, to);
  }).toArray();
}


Array<Spani> makeOverlappingSpans(int sampleCount, int splitSize, double relStep) {
  double step = relStep*splitSize;
  int lastIndex = int(floor((sampleCount - splitSize)/step));
  int count = lastIndex+1;
  LineKM from(0, count-1, 0.0, sampleCount - splitSize);
  LineKM to(0, count-1, splitSize, sampleCount);
  return map(Spani(0, count), [=](int i) {
    return Spani(int(round(from(i))), int(round(to(i))));
  }).toArray();
}


Eigen::MatrixXd makeConstantFlowTrajectoryMatrix(int observationCount) {
  using namespace DataFit;
  auto rows = CoordIndexer::Factory();
  auto cols = CoordIndexer::Factory();
  std::vector<Triplet> triplets;
  makeConstantFlowTrajectoryMatrix(rows.make(observationCount, 2), cols.make(2, 2),
      &triplets);
  Eigen::SparseMatrix<double> A(rows.count(), cols.count());
  A.setFromTriplets(triplets.begin(), triplets.end());
  return A.toDense();
}

// of a constant flow: That is a straight line, typically in two dimensions.
void makeConstantFlowTrajectoryMatrix(DataFit::CoordIndexer rows,
                                          DataFit::CoordIndexer cols,
                                          std::vector<DataFit::Triplet> *dst);

Eigen::VectorXd fitConstantFlow(const Eigen::VectorXd &dst) {
  int n = getObservationCount(dst);
  auto A = makeConstantFlowTrajectoryMatrix(n);
  CHECK(A.rows() == dst.rows());
  Eigen::MatrixXd AtA = A.transpose()*A;
  Eigen::VectorXd X = AtA.lu().solve(A.transpose()*dst);
  return A*X;
}

Eigen::VectorXd fitDataToGps(Eigen::MatrixXd A, Eigen::VectorXd B,
  Arrayd X) {
  Eigen::VectorXd Xe = EigenUtils::arrayToEigen(X);
  int n = getObservationCount(A);
  auto trueFlow = makeConstantFlowTrajectoryMatrix(n);
  Eigen::MatrixXd K = (trueFlow.transpose()*trueFlow);
  Eigen::VectorXd tw = B + A*Xe;
  Eigen::VectorXd fitted = K.lu().solve(trueFlow.transpose()*tw);
  CHECK(trueFlow.rows() == A.rows());
  CHECK(trueFlow.cols()*fitted.rows());
  CHECK(fitted.cols() == 1);
  CHECK(A.cols() == Xe.rows());
  CHECK(Xe.cols() == 1);
  return trueFlow*fitted - A*Xe;
}

namespace {
  using namespace DataFit;

  struct FitData {
    Eigen::MatrixXd A;

    Eigen::VectorXd B;

    irls::BinaryConstraintGroup cst;

    // So that we know what residuals to extract
    CoordIndexer flowRows;

    // So that we know what to set to zero when we compute the fitted wind.
    CoordIndexer dataSlackCols;
  };

  int countObservations(Array<Spani> spans) {
    return reduce(map(spans, [](Spani x) {return x.width();}), [](int a, int b) {return a + b;});
  }

  FitData makeConstantFlowFit(
          const Eigen::MatrixXd &Asub, const Eigen::MatrixXd &Bsub,
          CoordIndexer dataRows, CoordIndexer dataSlackRows,
          CoordIndexer outlierRows, CoordIndexer outlierSlackRows,
          CoordIndexer paramCols, CoordIndexer trueFlowCols, CoordIndexer dataSlackCols,
          CoordIndexer outlierSlackCols, std::vector<Triplet> *triplets,
          VectorBuilder *Bbuilder) {

    ///////// Data related
    // Left-hand-side
    insertDense(-1.0, Asub, dataRows.elementSpan(), paramCols.elementSpan(), triplets);
    makeConstantFlowTrajectoryMatrix(dataRows, trueFlowCols, triplets);
    makeEye(1.0, dataRows.elementSpan(), dataSlackCols.elementSpan(), triplets);
    makeEye(1.0, dataSlackRows.elementSpan(), dataSlackCols.elementSpan(), triplets);
    // Right-hand-side
    Bbuilder->add(dataRows.elementSpan(), Bsub);

    ///////// Outlier related
    // Related to outlier
    makeEye(1.0, outlierRows.elementSpan(), outlierSlackCols.elementSpan(), triplets);
    makeEye(1.0, outlierSlackRows.elementSpan(), outlierSlackCols.elementSpan(), triplets);

    auto cstFlowFit = fitConstantFlow(Bsub);
    auto constantFitError = (cstFlowFit - Bsub).norm();
    std::cout << EXPR_AND_VAL_AS_STRING(constantFitError) << std::endl;
    auto outlierResidual = constantFitError;
    Bbuilder->add(outlierRows.elementSpan(), outlierResidual);

    return FitData{Asub, Bsub, irls::BinaryConstraintGroup(dataSlackRows.elementSpan(),
        outlierSlackRows.elementSpan()), dataRows, dataSlackCols};
  }

  irls::WeightingStrategy::Ptr makeBinaryConstraints(Array<FitData> src) {
    int n = src.size();
    auto constraints = toArray(map(src, [&](const FitData &x) {
      return x.cst;
    }));
    return irls::WeightingStrategyArray<irls::BinaryConstraintGroup>::make(constraints);
  }

  void setTo0(CoordIndexer idx, Eigen::VectorXd *dst) {
    for (auto i: idx.elementSpan()) {
      (*dst)[i] = 0.0;
    }
  }

  LocallyConstantResults makeLocallyConstantResults(
      Eigen::SparseMatrix<double >A,
      Eigen::VectorXd Btrajectory,
      irls::Results results,
      Array<FitData> fitData,
      CoordIndexer paramCols) {
    int n = fitData.size();
    Arrayb inliers(n);
    Array<Eigen::VectorXd> segments(n);
    Arrayd parameters = EigenUtils::vectorToArray(results.X)
      .slice(paramCols.from(), paramCols.to()).dup();
    for (int i = 0; i < n; i++) {
      auto f = fitData[i];
      auto inlier = f.cst.getBestFitIndex(results.residuals) == 0;
      inliers[i] = inlier;
      auto inlierSegment = fitDataToGps(f.A, f.B, parameters);
      segments[i] = inlierSegment;
      auto outlierSegment = fitConstantFlow(f.B);

      auto s = (inlier? inlierSegment : outlierSegment);
      auto dif = s - f.B;
      auto squaredError = dif.squaredNorm()/sqr(dif.size());

    }
    return LocallyConstantResults{inliers, parameters, segments, Btrajectory};
  }
}

int LocallyConstantResults::inlierCount() const {
  int sum = 0;
  for (auto x: inliers) {
    sum += (x? 1 : 0);
  }
  return sum;
}

void LocallyConstantResults::plot() {
  TrajectoryPlot p;
  p.plot(B, 3, true);
  for (int i = 0; i < inliers.size(); i++) {
    p.plot(segments[i], (inliers[i]? 2 : 1), false);
  }
  p.show();
}

LocallyConstantResults optimizeLocallyConstantFlows(
    Eigen::MatrixXd Atrajectory, Eigen::VectorXd Btrajectory,
    Array<Spani> spans, const irls::Settings &settings) {
  using namespace DataFit;
  auto rows = CoordIndexer::Factory();
  auto cols = CoordIndexer::Factory();
  int observationCount = countObservations(spans);
  int finalRows = 2*observationCount;
  auto paramCols = cols.make(Atrajectory.cols(), 1);
  std::vector<Triplet> triplets;
  VectorBuilder Bbuilder;
  Array<FitData> data = map(spans, [&](Spani span) {
    Spani span2 = 2*span;
    auto dataRows = rows.make(span.width(), 2);
    auto dataSlackRows = rows.make(span.width(), 2);
    auto outlierRows = rows.make(1, 1);
    auto outlierSlackRows = rows.make(1, 1);
    auto trueFlowCols = cols.make(2, 2);
    auto dataSlackCols = cols.make(span.width(), 2);
    auto outlierSlackCols = cols.make(1, 1);
    auto Asub = sliceRows(Atrajectory, span2);
    auto Bsub = sliceRows(Btrajectory, span2);
    return makeConstantFlowFit(
            Asub, Bsub,
            dataRows, dataSlackRows, outlierRows, outlierSlackRows,
            paramCols, trueFlowCols, dataSlackCols, outlierSlackCols,
            &triplets, &Bbuilder);
  }).toArray();
  auto cst = makeBinaryConstraints(data);
  auto A = makeSparseMatrix(rows.count(), cols.count(), triplets);
  auto B = Bbuilder.make(rows.count());
  auto strategies = irls::WeightingStrategies{cst};
  irls::Results results = irls::solveFull(A, B, strategies, settings);
  return makeLocallyConstantResults(A, Btrajectory, results, data, paramCols);
}

DataFit::CoordIndexer makeSrcIndexer(int observationCount, int segmentSize) {
  int count = observationCount/segmentSize;
  int dim = 2;
  return DataFit::CoordIndexer::Factory().make(count, dim*segmentSize);
}

void makeFirstOrderSplineCoefs(DataFit::CoordIndexer dataRows,
                               DataFit::CoordIndexer splineCoefCols,
                               std::vector<DataFit::Triplet> *dst) {
  int dim = splineCoefCols.dim();
  int segmentCount = splineCoefCols.count() - 1;
  CHECK(dataRows.count() == segmentCount);
  int segmentSize = dataRows.dim()/dim;
  int segmentRows = dataRows.dim();

  double from = -0.5;
  double to = segmentSize-1 + 0.5;
  LineKM Amap(from, to, 1, 0);
  LineKM Bmap(from, to, 0, 1);
  for (int i = 0; i < segmentCount; i++) {
    Spani aSpan = splineCoefCols.span(i + 0);
    Spani bSpan = splineCoefCols.span(i + 1);
    int offset = i*segmentRows;
    for (int j = 0; j < segmentSize; j++) {
      auto a = Amap(j);
      auto b = Bmap(j);
      int rowOffset = offset + j*dim;
      Spani rowSpan(rowOffset, rowOffset + dim);
      for (int k = 0; k < dim; k++) {
        dst->push_back(Triplet(rowSpan[k], aSpan[k], a));
        dst->push_back(Triplet(rowSpan[k], bSpan[k], b));
      }
    }
  }
}

Array<Spani> makeOutlierSegmentData(DataFit::CoordIndexer constraintRows,
                                    DataFit::CoordIndexer splineCoefCols,
                                    DataFit::CoordIndexer outlierSlackCols,
                                    std::vector<DataFit::Triplet> *dst) {
  CHECK(constraintRows.dim() == splineCoefCols.dim() + 1);
  CHECK(outlierSlackCols.dim() == 1);
  CHECK(constraintRows.count() + 2 == splineCoefCols.count());
  CHECK(constraintRows.count() == outlierSlackCols.count());
  int dim = splineCoefCols.dim();
  int count = constraintRows.count();
  Array<Spani> spans(count);
  for (int i = 0; i < count; i++) {
    auto rowSpan = constraintRows.span(i);
    auto a = splineCoefCols.span(i + 0);
    auto b = splineCoefCols.span(i + 1);
    auto c = splineCoefCols.span(i + 2);
    for (int j = 0; j < dim; j++) {
      dst->push_back(Triplet(rowSpan[j], a[j],  1.0));
      dst->push_back(Triplet(rowSpan[j], b[j], -2.0));
      dst->push_back(Triplet(rowSpan[j], c[j],  1.0));
    }
    dst->push_back(Triplet(rowSpan[dim], outlierSlackCols[i], 1.0));
    spans[i] = rowSpan;
  }
  return spans;
}

Array<Spani> makeOutlierPenalty(
    DataFit::CoordIndexer srcDataSegments,
    Eigen::VectorXd srcGpsData,
    DataFit::CoordIndexer outlierPenaltyRows,
    DataFit::CoordIndexer outlierSlackCols,
    std::vector<DataFit::Triplet> *dst, VectorBuilder *B, int dim) {
  CHECK(srcDataSegments.numel() % dim == 0);
  CHECK(srcDataSegments.count() - 1 == outlierPenaltyRows.count());
  CHECK(outlierPenaltyRows.count() == outlierSlackCols.count());
  CHECK(outlierPenaltyRows.dim() == 1);
  CHECK(outlierSlackCols.dim() == 1);
  auto count = outlierPenaltyRows.count();
  Array<Spani> spans(count);
  for (int i = 0; i < count; i++) {
    auto b = EigenUtils::sliceRows(
        srcGpsData,
        srcDataSegments.from(i),
        srcDataSegments.to(i+1));
    auto fitted = fitConstantFlow(b);
    auto error = (b - fitted).norm();
    auto dstSpan = outlierPenaltyRows.span(i);
    spans[i] = dstSpan;
    B->add(dstSpan, error);
    dst->push_back(Triplet(outlierPenaltyRows[i], outlierSlackCols[i], 1.0));
  }
  return spans;
}

Eigen::MatrixXd applySecondOrderReg(const Eigen::MatrixXd &A, int step, int dim) {
  using namespace EigenUtils;
  DataFit::CoordIndexer rows = DataFit::CoordIndexer::Factory().makeFromSize(A.rows(), dim);
  int regCount = rows.count() - 2*step;
  int cols = A.cols();
  Eigen::MatrixXd dst(dim*regCount, cols);
  for (int i = 0; i < regCount; i++) {
    int a = i;
    int b = i + step;
    int c = b + step;
    sliceRows(dst, rows.span(i)) = sliceRows(A, rows.span(a))
        - 2.0*sliceRows(A, rows.span(b))
        + sliceRows(A, rows.span(c));
  }
  return dst;
}

Arrayd computeNorms(const Eigen::VectorXd &X, int dim) {
  auto rows = DataFit::CoordIndexer::Factory().makeFromSize(X.rows(), dim);
  Arrayd Y(rows.count());
  for (int i = 0; i < rows.count(); i++) {
    Y[i] = EigenUtils::sliceRows(X, rows.span(i)).norm();
  }
  return Y;
}

Arrayd computeWeightsFromGps(Eigen::MatrixXd B, int dim) {
  int n = B.rows()/dim;
  CHECK(dim*n == B.rows());
  auto norms = map(Spani(0, n), [=](int index) {
    int offset = dim*index;
    return EigenUtils::sliceRows(B, offset, offset + dim).norm();
  });
  double mean = (1.0/n)*reduce(norms, [](double a, double b) {return a + b;});
  return map(norms, [&](double x) {return x - mean;}).toArray();
}

void CovResults::plot() const {
  auto gpsReg = computeNorms(B, 2);
  auto flowReg = computeNorms(A*X + B, 2);

  auto inliers = inlierMask;
  auto outliers = sail::map(inliers, [](bool x) {return !x;}).toArray();


  GnuplotExtra plot;
  plot.setEqualAxes();
  plot.plot_xy(flowReg.slice(inliers), gpsReg.slice(inliers));
  plot.plot_xy(flowReg.slice(outliers), gpsReg.slice(outliers));
  plot.show();
}

Arrayd differentiate(Arrayd X) {
  return map(X.sliceFrom(1), X.sliceBut(1), [](double a, double b) {return a - b;}).toArray();
}

Arrayd differentiate(Arrayd X, int depth) {
  if (depth == 0) {
    return X;
  } else {
    return differentiate(differentiate(X), depth-1);
  }
}


void CovResults::plotDerivatives() const {
  auto gpsReg = differentiate(computeNorms(B, 2), 1);
  auto flowReg = differentiate(computeNorms(A*X + B, 1), 1);

  GnuplotExtra::Settings settings;
  settings.pointType = 9;


  GnuplotExtra plot;
  plot.setLineStyle(1, settings);
  plot.set_current_line_style(1);
  plot.setEqualAxes();
  plot.plot_xy(flowReg, gpsReg);
  plot.show();
}

CovResults optimizeCovariances(Eigen::MatrixXd Atrajectory,
                               Eigen::MatrixXd Btrajectory,
                               CovSettings settings) {
  auto Areg = applySecondOrderReg(Atrajectory, settings.regStep, 2);
  auto Breg = applySecondOrderReg(Btrajectory, settings.regStep, 2);
  /*auto weights = computeWeightsFromGps(Breg, 2);
  int regCount = Areg.rows()/2;
  CHECK(regCount == weights.size());
  auto rows = DataFit::CoordIndexer::Factory();
  auto cols = DataFit::CoordIndexer::Factory();
  std::vector<DataFit::Triplet> Atriplets;
  DataFit::VectorBuilder Bbuilder;
  auto regRows = CoordIndexer::Factory().makeFromSize(Areg.rows(), 2);
  auto paramCols = cols.make(Areg.cols(), 1);
  auto meanRegCol = cols.make(1, 1).from();
  auto slackCols = cols.make(regCount, 1);
  auto normRows = rows.make(regCount, 2);
  auto expectedLengthRows = rows.make(regCount, 1);
  auto slackRows = rows.make(regCount, 1);
  Array<irls::FitNorm> fnorms(regCount);
  Array<Spani> cstSpans(regCount);
  for (int i = 0; i < regCount; i++) {
    double w = weights[i];
    Spani xSpan = normRows.span(i);
    auto srcRows = regRows.span(i);
    insertDense(w, EigenUtils::sliceRows(Areg, srcRows), xSpan, paramCols.elementSpan(),
      &Atriplets);
    Bbuilder.add(xSpan, -w*EigenUtils::sliceRows(Breg, srcRows));
    int aRow = expectedLengthRows[i];
    int slackCol = slackCols[i];
    Atriplets.push_back(Triplet(aRow, slackCol, 1.0));
    Atriplets.push_back(Triplet(aRow, meanRegCol, 1.0));
    Atriplets.push_back(Triplet(slackRows[i], slackCol, 1.0));
    fnorms[i] = irls::FitNorm(xSpan, aRow, 2, false);
    cstSpans[i] = slackRows.span(i);
  }
  int activeCount = int(round(settings.inlierFraction*regCount));
  auto groupConstraints = new irls::ConstraintGroup(cstSpans, activeCount);
  irls::WeightingStrategies strategies{
    irls::WeightingStrategy::Ptr(new irls::WeightingStrategyArray<irls::FitNorm>(fnorms)),
    irls::WeightingStrategy::Ptr(groupConstraints)
  };
  auto results = irls::solveFull(makeSparseMatrix(rows.count(), cols.count(), Atriplets),
      Bbuilder.make(rows.count()), strategies, settings.irlsSettings);
  Arrayi inlierRegs = groupConstraints->computeActiveSpans(results.residuals);
  auto inlierMask = Arrayb::fill(regCount, false);
  for (auto i: inlierRegs) {
    inlierMask[i] = true;
  }
  return CovResults{
    Areg,
    Breg,
    EigenUtils::sliceRows(results.X, paramCols.elementSpan()),
    inlierMask
  };*/
  Eigen::VectorXd X = Eigen::VectorXd::Zero(Areg.cols());
  X(0) = 1.0;
  return CovResults{
    Areg, Breg, X, Arrayb()
  };
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
  LineKM vals(0, rows.count()-1, 0.0, 2.0);
  for (int i = 0; i < rows.count(); i++) {
    auto rowSpan = rows.span(i);
    for (int j = 0; j < dim; j++) {
      using namespace DataFit;
      int row = rowSpan[j];
      dst->push_back(Triplet(row, variableColSpan[j], vals(i)));
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

TrajectoryPlot::TrajectoryPlot() {
  _plot.set_style("lines");
  _plot.setLineStyle(1, "red", 1);
  _plot.setLineStyle(2, "green", 1);
  _plot.setLineStyle(3, "blue", 1);
  _plot.setLineStyle(4, "red", 2);
  _plot.setLineStyle(5, "green", 2);
  _plot.setLineStyle(6, "blue", 2);
}

void TrajectoryPlot::plot(Eigen::VectorXd X, int lineType, bool thick) {
  CHECK(Spani(0, 3).contains(lineType-1));
  int n = getObservationCount(X);
  MDArray2d data(n, 2);
  for (int i = 0; i < n; i++) {
    int row = 2*i;
    data(i, 0) = X(row + 0);
    data(i, 1) = X(row + 1);
  }
  _plot.set_current_line_style(lineType + (thick? 3 : 0));
  _plot.plot(data);
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
    plot.set_current_line_style(1);
    plot.plot(d.makePlotData(A, scale));
    plot.set_current_line_style(2);
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
  return map(splits, [=](Arrayi split) {
    auto q = integrateFlowData(extractRows(Q, split, 2));
    auto b = integrateFlowData(extractRows(B, split, 2));
    return FlowFiber{q, b};
  }).toArray();
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
