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
#include <server/common/Functional.h>

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
    return toArray(map(coordinateSpan(), [&](int index) {
      return span(index);
    }));
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



// Make left- and right-hands-sides for fitting of a signal to observations.
template <int Dim>
void makeDataFromObservations(Array<Observation<Dim> > observations,
    CoordIndexer dataRows, CoordIndexer sampleCols, std::vector<Triplet> *aDst,
    Eigen::VectorXd *bDst) {
  assert(observations.size() == dataRows.count());
  assert(dataRows.dim() == Dim);
  assert(sampleCols.dim() == Dim);
  for (auto i: dataRows.coordinateSpan()) {
    const Observation<Dim> &obs = observations[i];
    auto rowSpan = dataRows.span(i);
    makeEye(obs.weights.lowerWeight, rowSpan, sampleCols.span(obs.weights.lowerIndex), aDst);
    makeEye(obs.weights.upperWeight, rowSpan, sampleCols.span(obs.weights.upperIndex()), aDst);
    for (int j = 0; j < Dim; j++) {
      auto x = obs.data[j];
      assert(std::isfinite(x));
      (*bDst)(rowSpan[j]) = x;
    }
  }
}


irls::WeightingStrategy::Ptr makeThresholdedInlierFilter(double threshold,
    CoordIndexer dataRows,
    CoordIndexer inlierSlackRows,
    CoordIndexer outlierSlackRows,
    CoordIndexer outlierPenaltyRows,

    CoordIndexer sampleCols,
    CoordIndexer inlierCols,
    CoordIndexer outlierCols,

    std::vector<Triplet> *aDst, Eigen::VectorXd *bDst);




struct Results {
  Arrayb inliers;
  MDArray2d samples; // One reconstructed sample per row.
};

Results assembleResults(irls::Results solution, CoordIndexer inlierSlack,
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

  // Allocate rows and columns
  auto dataRows = rows.make(obsCount, Dim);
  auto sampleCols = cols.make(sampleCount, Dim);
  auto inlierSlackRows = rows.make(obsCount, Dim);
  auto outlierSlackRows = rows.make(obsCount, 1);
  auto outlierPenaltyRows = rows.make(obsCount, 1);
  auto regRows = rows.make(sampleCount - regOrder, Dim);
  auto inlierCols = cols.make(obsCount, Dim);
  auto outlierCols = cols.make(obsCount, 1);

  // Where all the elements are stored
  std::vector<Triplet> triplets;
  Eigen::VectorXd B = Eigen::VectorXd::Zero(rows.count());

  // Regularization
  makeReg(regWeight, regOrder, regRows, sampleCols, &triplets);

  // Data
  makeDataFromObservations(observations,
      dataRows, sampleCols, &triplets,
      &B);

  // Outlier removal
  auto strategy = makeThresholdedInlierFilter(inlierThreshold,
      dataRows, inlierSlackRows, outlierSlackRows,
      outlierPenaltyRows, sampleCols, inlierCols,
      outlierCols, &triplets, &B);

  // Put it together and solve it
  Eigen::SparseMatrix<double> A(rows.count(), cols.count());
  A.setFromTriplets(triplets.begin(), triplets.end());
  auto solution = irls::solveFull(A, B, irls::WeightingStrategies{strategy}, irlsSettings);

  return assembleResults(solution, inlierSlackRows, sampleCols, inlierThreshold);
}


}
}

#endif
