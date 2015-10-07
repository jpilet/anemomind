/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_SPARSECURVEFIT_H_
#define SERVER_MATH_NONLINEAR_SPARSECURVEFIT_H_

#include <server/math/irls.h>
#include <server/common/logging.h>
#include <server/math/nonlinear/SignalUtils.h>
#include <server/common/MDArray.h>
#include <server/common/Span.h>
#include <Eigen/Sparse>
#include <vector>

namespace sail {
namespace DataFit {
typedef Eigen::Triplet<double> Triplet;



void makeEye(double weight, Spani rowSpan, Spani colSpan, std::vector<Triplet> *dst);

// Facilitates the computation of indices
// in vectors and matrices storing coordinates
class CoordIndexer {
 private:
  // Please use the Factory class to make instances
  CoordIndexer(int offset, int dim, int count) :
    _offset(offset), _dim(dim), _count(count) {}

 public:
  class Factory {
   public:
    Factory() : _counter(0) {}

    CoordIndexer make(int count, int dim) {
      int offset = _counter;
      _counter += dim*count;
      return CoordIndexer(offset, dim, count);
    }

    int count() const {
      return _counter;
    }
   private:
    int _counter;
  };

  int from() const {
    return _offset;
  }

  int from(int index) const {
    return _offset + index*_dim;
  }

  int to(int index) const {
    return from(index) + _dim;
  }

  int numel() const {
    return _dim*_count;
  }

  int to() const {
    return _offset + numel();
  }

  Spani elementSpan() const {
    return Spani(from(), to());
  }

  Spani coordinateSpan() const {
    return Spani(0, _count);
  }

  Spani span(int index) const {
    assert(0 <= index);
    assert(index < _count);
    int offset = _offset + index*_dim;
    return Spani(offset, offset + _dim);
  }

  int dim() const {
    return _dim;
  }

  int count() const {
    return _count;
  }

  Array<Spani> makeSpans() const {
    return coordinateSpan().map<Spani>([&](int index) {
      return span(index);
    });
  }

  bool sameSizeAs(CoordIndexer other) const {
    return _dim == other._dim && _count == other._count;
  }

  int operator[] (int i) const {
    assert(_dim == 1);
    return _offset + i;
  }
 private:
  int _offset, _dim, _count;
};


// Make regularization term
void makeReg(double weight, int order,
    CoordIndexer rowIndexer, CoordIndexer colIndexer,
    std::vector<Triplet> *dst);


// Makes the part of the problem related to
// fitting to data that contains outliers.
// A threshold is used to classify points as being outliers.
template <int Dim>
irls::WeightingStrategy::Ptr makeDataWithOutliers(double threshold,
    Array<Observation<Dim> > observations,

    CoordIndexer dataRows,
    CoordIndexer inlierSlackRows,
    CoordIndexer outlierSlackRows,
    CoordIndexer outlierPenaltyRows,

    CoordIndexer dataCols,
    CoordIndexer inlierCols,
    CoordIndexer outlierCols,

    std::vector<Triplet> *aDst, Eigen::VectorXd *bDst) {
  int count = observations.size();
  assert(observations.size() == dataRows.count());
  assert(dataRows.dim() == dataCols.dim());
  assert(inlierSlackRows.sameSizeAs(dataRows));
  assert(inlierSlackRows.sameSizeAs(inlierCols));
  assert(outlierSlackRows.sameSizeAs(outlierCols));
  assert(inlierSlackRows.count() == outlierSlackRows.count());
  assert(outlierSlackRows.dim() == 1);
  assert(outlierPenaltyRows.sameSizeAs(outlierSlackRows));

  Array<irls::ConstraintGroup> groups(count);
  for (int i = 0; i < count; i++) {
    const Observation<Dim> &obs = observations[i];
    auto rowSpan = dataRows.span(i);
    auto inlierRowSpan = inlierSlackRows.span(i);
    makeEye(obs.weights.lowerWeight, rowSpan, dataCols.span(obs.weights.lowerIndex), aDst);
    makeEye(obs.weights.upperWeight, rowSpan, dataCols.span(obs.weights.upperIndex()), aDst);
    for (int j = 0; j < Dim; j++) {
      (*bDst)(rowSpan[j]) = obs.data[j];
      (*bDst)(inlierRowSpan[j]) = 0.0;
    }
    makeEye(1.0, rowSpan, inlierCols.span(i), aDst);
    makeEye(1.0, inlierSlackRows.span(i), inlierCols.span(i), aDst);
    (*bDst)(outlierPenaltyRows[i]) = threshold;
    (*bDst)(outlierSlackRows[i]) = 0;
    groups[i] = irls::ConstraintGroup(
        Array<Spani>{inlierSlackRows.span(i), outlierSlackRows.span(i)}, 1);
  }
  makeEye(1.0, outlierPenaltyRows.elementSpan(), outlierCols.elementSpan(), aDst);
  makeEye(1.0, outlierSlackRows.elementSpan(), outlierCols.elementSpan(), aDst);
  return irls::WeightingStrategyArray<irls::ConstraintGroup>::make(groups);
}









Array<Spani> makeReg(int order, int firstRowOffset, int firstColOffset,
    int dim, int count, std::vector<Triplet> *dst);

template <int Dim>
Array<Spani> makeDataFitness(int firstRowOffset, int firstColOffset,
    Array<Observation<Dim> > observations, int sampleCount,
    std::vector<Triplet> *dst, Eigen::VectorXd *rhs) {
  int count = observations.size();
  int totalDim = Dim*count;
  int sampleDim = Dim*sampleCount;

  for (int i = 0; i < count; i++) {
    Observation<Dim> x = observations[i];
    int colA = firstColOffset + Dim*x.weights.lowerIndex;
    int colB = firstColOffset + Dim*x.weights.upperIndex();
    int iDim = i*Dim;
    int rowOffset = firstRowOffset + iDim;
    for (int k = 0; k < Dim; k++) {
      int row = rowOffset + k;

      // Express a point on the curve as a linear combination of the samples.
      dst->push_back(Triplet(row, colA + k, x.weights.lowerWeight));
      dst->push_back(Triplet(row, colB + k, x.weights.upperWeight));

      (*rhs)(row) = x.data[k];
      int slackCol = firstColOffset + sampleDim + iDim + k;

      // On the same row as the observation,
      // there is also a slack variable. For inliers,
      // this slack variable will be zero. For outliers,
      // it can be anything that minimizes the datafit.
      // The algorithm will automatically select the best
      // choice of slack variables to be put to zero.
      dst->push_back(Triplet(row, slackCol, 1.0));

      // This row is part of the constraint, that
      // forces some slack variables to zero.
      dst->push_back(Triplet(row + totalDim, slackCol, 1.0));
    }
  }
  return Spani(0, count).map<Spani>([&](int index) {
    int offset = firstRowOffset + totalDim + Dim*index;
    return Spani(offset, offset + Dim);
  });
}



struct Settings {
  // These are the settings of the underlying algorithm used to solve the problem.
  irls::Settings spcstSettings;

  // Select this fraction of all observations to use as inliers, ignore the others.
  // An alternative to this would have been to have some inlier threshold, sigma, so that
  // if the distance between the observation and the fitted curve is less than sigma, then
  // the observation should be treated as an inlier. In that case, we should know more or less
  // the noise distribution, but we might not know it. The problem with choosing sigma is that
  // a value that is too low will make us overfit to a few samples, whereas a value which is too high
  // will also include outliers among the samples treated as inliers.
  // On the other hand, when picking
  // a fraction of all the measurements and use to perform the fit, we should be sure that the inlier
  // rate is not lower than that fraction, because then it is going to break. And we should not choose
  // the fraction too low, because then we are going to overfit to a few samples. In either case,
  // no matter if we choose fraction-based outlier rejection or threshold-based rejection, we will
  // have to tune a parameter because one size doesn't fit all. But in most cases, I believe that it
  // is fair to say that at least around the best 60% of the samples should be high quality
  // observations even for crappy gps devices.
  double inlierRate = 0.6;

  // For which derivative order we count the discontinuities.
  // A regOrder of 1 would mean a constant piecewise curve.
  // A regOrder of 2 means a continuous curve of piecewise straight line segments,
  //   or its first derivative being piecewise constant.
  // A regorder of 3 means that the second derivative of the curve is piecewise constant.
  int regOrder = 3;


  // Put a bound on the norm of the difference vector from one sample
  // to the next. A negative bound means no bound.
  double maxNorm = -1;
};



struct Results {
  Arrayb inliers;
  MDArray2d samples; // One reconstructed sample per row.
};

Results assembleResults(irls::Results solution, CoordIndexer data,
    CoordIndexer samples, double inlierThreshold);


/*
 * Just very simple fit to data with quadratic regularization.
 * Inlier data points are fitted quadratically, outlier ones are completely ignored.
 * The inlierThreshold parameter controls the maximum distance between the fitted
 * curve and the data.
 */
template <int Dim>
Results quadraticFitWithInliers(
    int sampleCount,
    Array<Observation<Dim> > observations,
    double inlierThreshold,
    int regOrder, double regWeight, irls::Settings irlsSettings) {
  int obsCount = observations.size();
  CoordIndexer::Factory rows, cols;
  auto dataRows = rows.make(obsCount, Dim);
  auto sampleCols = cols.make(sampleCount, Dim);
  auto inlierSlackRows = rows.make(obsCount, Dim);
  auto outlierSlackRows = rows.make(obsCount, 1);
  auto outlierPenaltyRows = rows.make(obsCount, 1);
  auto regRows = rows.make(sampleCount - regOrder, Dim);
  auto inlierCols = cols.make(obsCount, Dim);
  auto outlierCols = cols.make(obsCount, 1);
  std::vector<Triplet> triplets;
  Eigen::VectorXd B(rows.count());
  makeReg(regWeight, regOrder, regRows, sampleCols, &triplets);
  std::cout << EXPR_AND_VAL_AS_STRING(regRows.to()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(rows.count()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(sampleCols.to()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(cols.count()) << std::endl;
  auto last = triplets.back();
  std::cout << EXPR_AND_VAL_AS_STRING(last.row()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(last.col()) << std::endl;

  auto strategy = makeDataWithOutliers(inlierThreshold, observations,
      dataRows, inlierSlackRows, outlierSlackRows,
      outlierPenaltyRows, sampleCols, inlierCols,
      outlierCols, &triplets, &B);
  Eigen::SparseMatrix<double> A(rows.count(), cols.count());
  A.setFromTriplets(triplets.begin(), triplets.end());
  auto solution = irls::solveFull(A, B, irls::WeightingStrategies{strategy}, irlsSettings);
  return assembleResults(solution, dataRows, sampleCols, inlierThreshold);
}



Results assembleResults(int dim, int sampleCount, int inlierCount, const Eigen::VectorXd &solution);

/*
 *
 * The main function to solve the problem.
 *
 */
template <int Dim>
Results fit(int sampleCount, int discontinuityCount,
    Array<Observation<Dim> > observations, const Settings &settings) {
  CHECK(0 <= discontinuityCount);
  CHECK(observations.all([&](const Observation<Dim> &x) {
    return 0 <= x.weights.lowerIndex && x.weights.upperIndex() < sampleCount;
  }));

  bool withBound = settings.maxNorm > 0;

  int boundCount = (withBound? (sampleCount - 1) : 0);
  int boundRowCount = Dim*boundCount;

  int regCount = (sampleCount - settings.regOrder);
  int coefsPerReg = (settings.regOrder + 1);
  int regRowCount = Dim*regCount;
  int regElemCount = coefsPerReg*regRowCount;
  int dataElemCount = Dim*4*observations.size();
  int dataRowCount = observations.size()*Dim*2;
  int rowCount = dataRowCount + regRowCount + boundRowCount;

  Eigen::VectorXd rhs = Eigen::VectorXd::Zero(rowCount);
  std::vector<Triplet> elements;
  elements.reserve(dataElemCount + regElemCount);
  Array<Spani> slackSpans = makeDataFitness(0, 0,
      observations, sampleCount,
      &elements, &rhs);
  CHECK(slackSpans.last().maxv() == dataRowCount);
  Array<Spani> regSpans = makeReg(settings.regOrder, dataRowCount, 0,
      Dim, regCount, &elements);
  CHECK(elements.back().col() <= Dim*sampleCount);
  std::cout << EXPR_AND_VAL_AS_STRING(dataRowCount) << std::endl;
  CHECK(regSpans.first().minv() == dataRowCount);
  int dataAndRegRowCount = dataRowCount + regRowCount;
  CHECK(regSpans.last().maxv() == dataAndRegRowCount);
  Array<Spani> boundSpans(boundCount);
  if (withBound) {
    boundSpans = makeReg(1, dataAndRegRowCount, 0, Dim, boundCount, &elements);
  }

  int inlierCount = int(round(observations.size()*settings.inlierRate));
  CHECK(0 <= discontinuityCount);
  int activeRegCount = regCount - discontinuityCount;

  irls::WeightingStrategies strategies{
    irls::WeightingStrategy::Ptr(new irls::ConstraintGroup(slackSpans, inlierCount)),
    irls::WeightingStrategy::Ptr(new irls::ConstraintGroup(regSpans, activeRegCount)),
    irls::BoundedNormConstraint::make(boundSpans, settings.maxNorm)
  };

  int colCount = Dim*sampleCount + Dim*observations.size();
  Eigen::SparseMatrix<double> lhs(rowCount, colCount);
  lhs.setFromTriplets(elements.begin(), elements.end());
  Eigen::VectorXd solution =
      irls::solve(lhs, rhs, strategies, settings.spcstSettings);
  CHECK(solution.size() == colCount);
  return assembleResults(Dim, sampleCount, inlierCount, solution);
}


}
}

#endif /* SERVER_MATH_NONLINEAR_SPARSECURVEFIT_H_ */
