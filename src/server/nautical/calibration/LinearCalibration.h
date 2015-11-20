/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEARCALIBRATION_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>
#include <server/math/irls.h>
#include <server/math/nonlinear/DataFit.h>
#include <server/math/Random.h>
#include <server/plot/extra.h>

namespace sail {
namespace LinearCalibration {

void initializeParameters(bool withOffset, double *dst);
Eigen::VectorXd makeXinitEigen();

inline int flowParamCount(bool withOffset) {
  return (withOffset? 4 : 2);
}

struct FlowSettings {
 bool windWithOffset = true;
 bool currentWithOffset = true;

 int windParamCount() const {
   return flowParamCount(windWithOffset);
 }

 int currentParamCount() const {
   return flowParamCount(currentWithOffset);
 }
};


/*
 * See docs/calib/linearcalib.tex
 *
 * If withOffset is false, dst is assumed to be a 2x2 matrix,
 * otherwise a 2x4 matrix. The dst matrix, when multiplied with
 * a parameter vector, results in a calibrated motion.
 */
template <typename MatrixType>
void makeCalibratedMotionMatrix(
    Angle<double> angle, Velocity<double> magnitude,
    bool withOffset,
    MatrixType *dst, Velocity<double> unit) {
  double cosPhi = cos(angle);
  double sinPhi = sin(angle);
  double r = magnitude/unit;
  (*dst)(0, 0) = r*sinPhi; (*dst)(0, 1) = -r*cosPhi;
  (*dst)(1, 0) = r*cosPhi; (*dst)(1, 1) = r*sinPhi;
  if (withOffset) {
    (*dst)(0, 2) = sinPhi; (*dst)(0, 3) = -cosPhi;
    (*dst)(1, 2) = cosPhi; (*dst)(1, 3) = sinPhi;
  }
}

template <typename InstrumentAbstraction>
HorizontalMotion<double> getGpsMotion(const InstrumentAbstraction &nav) {
  return HorizontalMotion<double>::polar(nav.gpsSpeed(), nav.gpsBearing());
}

template <typename MatrixType>
void makeGpsOffset(const HorizontalMotion<double> &m, MatrixType *dstB,
    Velocity<double> unit) {
  (*dstB)(0, 0) = m[0]/unit;
  (*dstB)(1, 0) = m[1]/unit;
}

/*
 * Use this function to express the true wind W as a function of the parameters X:
 *
 * W(X) = AX + B
 *
 * If withOffset = false, then A is 2x2, otherwise if it is true, then A is 2x4
 * B is always 2x1
 */
template <typename InstrumentAbstraction, typename MatrixType>
void makeTrueWindMatrixExpression(const InstrumentAbstraction &nav,
  FlowSettings settings,
  MatrixType *dstA, MatrixType *dstB,
  Velocity<double> unit = Velocity<double>::knots(1.0)) {
  auto absoluteDirectionOfWind = nav.magHdg() + nav.awa() + Angle<double>::degrees(180);
  makeCalibratedMotionMatrix(absoluteDirectionOfWind, nav.aws(), settings.windWithOffset, dstA, unit);
  makeGpsOffset(getGpsMotion(nav), dstB, unit);
}


struct FlowMatrices {
 MDArray2d A, B;

 int rows() const {
   assert(A.rows() == B.rows());
   return A.rows();
 }

 int count() const {
   return rows()/2;
 }
};

template <typename InstrumentAbstraction>
FlowMatrices makeTrueWindMatrices(Array<InstrumentAbstraction> navs, const FlowSettings &s) {
  int n = navs.size();
  int paramCount = s.windParamCount();
  MDArray2d A(2*n, paramCount);
  MDArray2d B(2*n, 1);
  for (int i = 0; i < n; i++) {
    int offset = 2*i;
    auto a = A.sliceRowBlock(i, 2);
    auto b = B.sliceRowBlock(i, 2);
    makeTrueWindMatrixExpression(navs[i], s, &a, &b);
  }
  return FlowMatrices{A, B};
}

template <typename InstrumentAbstraction>
FlowMatrices makeTrueCurrentMatrices(Array<InstrumentAbstraction> navs, const FlowSettings &s) {
  int n = navs.size();
  int paramCount = s.currentParamCount();
  MDArray2d A(2*n, paramCount);
  MDArray2d B(2*n, 1);
  for (int i = 0; i < n; i++) {
    int offset = 2*i;
    auto a = A.sliceRowBlock(i, 2);
    auto b = B.sliceRowBlock(i, 2);
    makeTrueCurrentMatrixExpression(navs[i], s, &a, &b);
  }
  return FlowMatrices{A, B};
}


/*
 * Use this function to express the true current C as a function of the parameters X:
 *
 * C(X) = AX + B
 *
 * If withOffset = false, then A is 2x2, otherwise if it is true, then A is 2x4
 * B is always 2x1
 */
template <typename InstrumentAbstraction, typename MatrixType>
void makeTrueCurrentMatrixExpression(const InstrumentAbstraction &nav,
  const FlowSettings &s,
  MatrixType *dstA, MatrixType *dstB,
  Velocity<double> unit = Velocity<double>::knots(1.0)) {
  auto oppositeDirectionOfBoatOverWater = nav.magHdg() + Angle<double>::degrees(180);
  makeCalibratedMotionMatrix(oppositeDirectionOfBoatOverWater,
      nav.watSpeed(), s.currentWithOffset, dstA, unit);
  makeGpsOffset(getGpsMotion(nav), dstB, unit);
}


/*
 * Perform full calibration of wind and current.
 */
// A class used to map a raw nav to a corrected one, using the parameters recovered.
class LinearCorrector : public CorrectorFunction {
 public:
  LinearCorrector() {}
  LinearCorrector(const FlowSettings &flowSettings, Arrayd windParams, Arrayd currentParams);
  Array<CalibratedNav<double> > operator()(const Array<Nav> &navs) const;
  CalibratedNav<double> operator()(const Nav &navs) const;
  std::string toString() const;
 private:
  FlowSettings _flowSettings;
  Arrayd _windParams, _currentParams;
};


template <typename MatrixType>
int getObservationCount(const MatrixType &X) {
  int count = X.rows()/2;
  CHECK(2*count == X.rows());
  return count;
}

struct CalibrationSettings {
  irls::Settings irlsSettings;
  int samplesPerSpan = 60;
  double inlierFrac = 0.2;
};


struct SubtractMeanResults {
  Eigen::MatrixXd results;
  Eigen::MatrixXd mean;
};
Eigen::MatrixXd subtractMean(Eigen::MatrixXd A, int dim);
Eigen::MatrixXd integrate(Eigen::MatrixXd A, int dim);
Eigen::MatrixXd integrateFlowData(Eigen::MatrixXd X);

struct FlowFiber {
  Eigen::MatrixXd Q, B;

  MDArray2d makePlotData(Eigen::VectorXd params, double scale = 1.0);
  double eval(Eigen::VectorXd params, double scale = 1.0);

  int rows() const {
    assert(Q.rows() == B.rows());
    return Q.rows();
  }

  int observationCount() const {
    assert(Q.rows() % 2 == 0);
    return Q.rows()/2;
  }

  int parameterCount() const {
    return Q.cols();
  }

  FlowFiber differentiate() const;
  FlowFiber integrate() const;
  FlowFiber dropConstant() const;
  FlowFiber dropVariable() const;
  bool sameSizeAs(const FlowFiber &other) const;
  Eigen::VectorXd minimizeNorm() const;
};

class TrajectoryPlot {
 public:
  TrajectoryPlot();
  void plot(Eigen::VectorXd X, int lineType, bool thick);
  void show() {_plot.show();}
 private:
  GnuplotExtra _plot;
};


// Makes a matrix used to parametrize the trajectory
// of a constant flow: That is a straight line, typically in two dimensions.
void makeConstantFlowTrajectoryMatrix(DataFit::CoordIndexer rows,
                                          DataFit::CoordIndexer cols,
                                          std::vector<DataFit::Triplet> *dst);

FlowFiber operator+(const FlowFiber &a, const FlowFiber &b);
FlowFiber operator-(const FlowFiber &a, const FlowFiber &b);
FlowFiber operator-(const FlowFiber &a);
FlowFiber operator*(double x, const FlowFiber &b);




void plotFlowFibers(Array<FlowFiber> flowFibers, Eigen::VectorXd params, double scale = 1.0);
void plotFlowFibers(Array<FlowFiber> data,
    Eigen::VectorXd A, Eigen::VectorXd B, double scale = 1.0);


Eigen::MatrixXd extractRows(Eigen::MatrixXd mat, Arrayi inds, int dim);
Array<FlowFiber> makeFlowFibers(Eigen::MatrixXd Q, Eigen::MatrixXd B,
    Array<Arrayi> splits);

Array<FlowFiber> computeFiberMeans(Array<FlowFiber> rawFibers, int dstCount);

FlowFiber computeMeanFiber(Array<FlowFiber> fibers);

// Build a fiber, whose norm |Q*X + B| should be as small as possible.
FlowFiber buildFitnessFiber(Array<FlowFiber> fibers, FlowFiber mean);

Eigen::VectorXd smallestEigVec(const Eigen::MatrixXd &K);
Array<Arrayi> makeRandomSplit(int sampleCount, int splitCount,
    RandomEngine *rng = nullptr);

Array<Spani> makeContiguousSpans(int sampleCount, int splitSize);
Array<Spani> makeOverlappingSpans(int sampleCount, int splitSize, double step = 0.5);

Eigen::VectorXd fitConstantFlow(const Eigen::VectorXd &dst);

struct LocallyConstantResults {
  Arrayb inliers;
  Arrayd parameters;
  Array<Eigen::VectorXd> segments;
  Eigen::VectorXd B;

  int inlierCount() const;
  double inlierRate() const {
    return double(inlierCount())/inliers.size();
  }
  void plot();
};

// TODO: Autoselect the span count so that the number of inliers is maximized.
LocallyConstantResults optimizeLocallyConstantFlows(
    Eigen::MatrixXd Atrajectory, Eigen::VectorXd Btrajectory,
    Array<Spani> spans, const irls::Settings &settings);




////////////// Next version
DataFit::CoordIndexer makeSrcIndexer(int observationCount, int segmentSize);

void makeFirstOrderSplineCoefs(DataFit::CoordIndexer segmentRows,
                               DataFit::CoordIndexer splineCoefCols,
                               std::vector<DataFit::Triplet> *dst);

Array<Spani> makeOutlierSegmentData(DataFit::CoordIndexer constraintRows,
                                    DataFit::CoordIndexer splineCoefCols,
                                    DataFit::CoordIndexer outlierSlackCols,
                                    std::vector<DataFit::Triplet> *dst);

Array<Spani> makeOutlierPenalty(
    DataFit::CoordIndexer srcDataSegments,
    Eigen::VectorXd srcGpsData,
    DataFit::CoordIndexer outlierPenaltyRows,
    DataFit::CoordIndexer outlierSlackCols,
    std::vector<DataFit::Triplet> *dst, DataFit::VectorBuilder *B, int dim = 2);


Eigen::MatrixXd applySecondOrderReg(const Eigen::MatrixXd &A, int step, int dim);

Arrayd computeNorms(const Eigen::VectorXd &X, int dim);

struct CovSettings {
  int regStep = 119;
  double inlierFraction = 0.9;
};

struct CovResults {
  Eigen::VectorXd X;
};

CovResults optimizeCovariances(Eigen::MatrixXd Atrajectory,
                               Eigen::MatrixXd Btrajectory,
                               CovSettings settings);



}
}

#endif
