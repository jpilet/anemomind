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

namespace {
  using namespace DataFit;

  Eigen::MatrixXd makeAvgMat(int count, int dim) {
    int rows = count*dim;
    int cols = dim;
    double f = 1.0/sqrt(count);
    Eigen::MatrixXd dst = Eigen::MatrixXd::Zero(rows, cols);
    for (int i = 0; i < count; i++) {
      int rowOffset = i*dim;
      for (int j = 0; j < dim; j++) {
        dst(rowOffset + j, j) = f;
      }
    }
    return dst;
  }

  Eigen::MatrixXd removeMeanAndConvertToEigen(MDArray2d src, int dim = 2) {
    assert(src.isContinuous());
    auto srcEigen = Eigen::Map<Eigen::MatrixXd>(src.ptr(), src.rows(), src.cols());
    int count = src.rows()/dim;
    assert(count*dim == src.rows());
    auto cst = makeAvgMat(count, dim);
    Eigen::MatrixXd dst = srcEigen - cst*(cst.transpose()*srcEigen);
    return dst;
  }

  Array<Spani> makeSpans(int n, int samplesPerSpan) {
    int spanCount = n/samplesPerSpan;
    return Spani(0, spanCount).map<Spani>([=](int i) {
      int offset = i*samplesPerSpan;
      return Spani(offset, offset + samplesPerSpan);
    });
  }

  void makeSpanData(MDArray2d A,
                    MDArray2d B,
                    CoordIndexer spanRows,
                    CoordIndexer slackConstraintRows,
                    CoordIndexer apparentFlowCols,
                    CoordIndexer slackCols,
                    std::vector<Triplet> *triplets,
                    Eigen::VectorXd *Bdst) {

    // The assumption is that the true flow (true wind or current)
    // is locally constant. So instead of trying to estimate it,
    // subract it, whatever it is. This is a linear operation.
    auto Aeigen = removeMeanAndConvertToEigen(A, 2);
    Eigen::VectorXd Beigen = removeMeanAndConvertToEigen(B, 2);

    // Normalize both the apparent flow and the GPS speed by the
    // norm of the GPS speed. So if the GPS speed is constant,
    // after subtracting its mean, it will be closed to 0.
    // Now, normalizing the vector will boost noise, whereas
    // if we perform a manoeuver, we would not boost noise, and
    // instead have an informative piece of data to fit to.
    auto bNorm = Beigen.norm();
    double f = 1.0/(1.0e-12 + std::abs(bNorm));

    // TrueFlow = ApparentFlow + GPS-speed
    // When we remove the mean, we have
    // removeMeanAndConvertToEigen(TrueFlow) = removeMeanAndConvertToEigen(ApparentFlow) + removeMeanAndConvertToEigen(GPS-speed)
    // <=> [ removeMeanAndConvertToEigen(TrueFlow) = 0 since it is locally constant] <=>
    // f*removeMeanAndConvertToEigen(ApparentFlow) = -f*removeMeanAndConvertToEigen(GPS-speed)
    // where f is a normalization factor.
    insertDense(f, Aeigen, spanRows.elementSpan(),
                apparentFlowCols.elementSpan(), triplets);
    Bdst->block(spanRows.from(), 0, spanRows.numel(), 1) = -f*Beigen;

    // Since we only want to use a subset of the spans that yield the best
    // fit, we also introducce slack variables. We have constraints that
    // force some slack variables to be 0 for the spans that we choose to use.
    // For the other spans, the slack variables can be anything that minimizes the
    // objective function.
    makeEye(1.0, spanRows.elementSpan(),
            slackCols.elementSpan(), triplets);
    makeEye(1.0, slackConstraintRows.elementSpan(),
            slackCols.elementSpan(), triplets);
  }

  MDArray2d sliceRows(MDArray2d A, Spani span, int dim) {
    Spani s = dim*span;
    return A.sliceRows(s.minv(), s.maxv());
  }

  irls::ConstraintGroup *makeInlierConstraints(double inlierFrac,
    Array<CoordIndexer> slackIndexers) {
    int n = slackIndexers.size();
    int active = int(ceil(inlierFrac*n));
    Array<Spani> spans = slackIndexers.map<Spani>([](CoordIndexer c) {
          return c.elementSpan();
        });;
    return new irls::ConstraintGroup(spans, active);
  }

}

PlotData makePlotData(CoordIndexer indexer, Eigen::VectorXd flow, Eigen::VectorXd gps) {
  int n = indexer.count();
  MDArray2d Xflow(n, 2), Yflow(n, 2), Xgps(n, 2), Ygps(n, 2);
  for (int i = 0; i < n; i++) {
    Xflow(i, 0) = i;
    Yflow(i, 0) = i;
    Xgps(i, 0) = i;
    Ygps(i, 0) = i;
    auto span = indexer.span(i);
    Xflow(i, 1) = flow(span[0]);
    Yflow(i, 1) = flow(span[1]);
    Xgps(i, 1) = gps(span[0]);
    Ygps(i, 1) = gps(span[1]);
  }
  return PlotData{Xflow, Yflow, Xgps, Ygps};
}

Results calibrate(FlowMatrices mats, const CalibrationSettings &s) {
  using namespace DataFit;
  assert(mats.A.rows() == mats.B.rows());
  int sampleCount = mats.A.rows()/2;
  assert(2*sampleCount == mats.A.rows());
  auto spans = makeSpans(sampleCount, s.samplesPerSpan);
  int spanCount = spans.size();

  auto rows = CoordIndexer::Factory();
  auto cols = CoordIndexer::Factory();

  int expectedRows = 4*s.samplesPerSpan*spans.size();
  Eigen::VectorXd B = Eigen::VectorXd::Zero(expectedRows);
  auto apparentFlowCols = cols.make(mats.A.cols(), 1);
  std::vector<Triplet> triplets;

  Array<CoordIndexer> allSpanRows(spanCount),
      allSlackRows(spanCount), allSlackCols(spanCount);
  for (int i = 0; i < spanCount; i++) {
    auto spanRows = rows.make(s.samplesPerSpan, 2);
    allSpanRows[i] = spanRows;

    auto slackRows = rows.make(s.samplesPerSpan, 2);
    allSlackRows[i] = slackRows;

    auto slackCols = cols.make(s.samplesPerSpan, 2);
    allSlackCols[i] = slackCols;

    auto span = spans[i];

    makeSpanData(sliceRows(mats.A, span, 2),
        sliceRows(mats.B, span, 2),
        spanRows, slackRows, apparentFlowCols, slackCols, &triplets, &B);
  }
  CHECK(expectedRows == rows.count());
  Eigen::SparseMatrix<double> A(rows.count(), cols.count());
  A.setFromTriplets(triplets.begin(), triplets.end());

  std::cout << EXPR_AND_VAL_AS_STRING(spanCount) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(allSlackRows.size()) << std::endl;

  auto rawCst = makeInlierConstraints(s.inlierFrac, allSlackRows);
  irls::WeightingStrategy::Ptr cst(rawCst);
  auto solution = irls::solveFull(A, B, irls::WeightingStrategies{cst}, s.irlsSettings);

  Arrayd X(solution.X.size(), solution.X.data());
  Arrayd parameters = X.slice(apparentFlowCols.from(), apparentFlowCols.to()).dup();

  auto inliers = rawCst->computeActiveSpans(solution.residuals);
  std::cout << EXPR_AND_VAL_AS_STRING(inliers) << std::endl;
  Arrayb mask = Arrayb::fill(spanCount, false);
  for (auto i : inliers) {
    CHECK(0 <= i && i < spanCount);
    mask[i] = true;
  }

  auto AX = irls::product(A, solution.X);

  return Results{
    parameters,
    spans,
    inliers,
    mask
  };
}


}
}
