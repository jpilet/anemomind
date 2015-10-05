/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_SPARSITYCONSTRAINED_H_
#define SERVER_MATH_SPARSITYCONSTRAINED_H_

#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <server/common/Array.h>
#include <server/common/Span.h>
#include <ceres/ceres.h>
#include <server/common/math.h>

namespace sail {
namespace irls {

/*
 * Performs Iteratively Reweighted Least Squares. This is the name of the practice
 * of solving least squares problems of type |W*(A*X - B)|^2, where W is a diagonal matrix
 * whose non-zero entries are updated for every iteration.
 */


// Minimize w.r.t. W: |diag(W)*sqrt(residuals)|^2 subject to average(W) = avgWeight.
// All residuals must be positive.
Arrayd distributeWeights(Arrayd residuals, double avgWeight);

struct Settings {
 int iters = 30;
 double initialWeight = 0.1;
 double finalWeight = 10000;
};

typedef Eigen::DiagonalMatrix<double, Eigen::Dynamic, Eigen::Dynamic> DiagMat;

// Manages weighting of several overlapping rows
class Weigher {
 public:
  Weigher(int dim) : _squaredWeights(Arrayd::fill(dim, -1.0)) {}

  void setSquaredWeight(int index, double squaredWeight) {
    assert(0 <= squaredWeight);
    if (isWeighted(index)) {
      _squaredWeights[index] += squaredWeight;
    } else { // Initialization:
      _squaredWeights[index] = squaredWeight;
    }
  }

  void setWeight(int index, double weight) {
    setSquaredWeight(index, sqr(weight));
  }

  double calcWeight(int index) const {
    auto w = _squaredWeights[index];
    // Any weight that is never set is assumed to not
    // be reweighted, and gets the weight 1.0.
    // Otherwise, take the square root of the squared weight sum.
    return (w < -0.5? 1.0 : sqrt(w));
  }

  DiagMat makeWeightMatrix() const;
 private:
  bool isWeighted(int index) const {
    return _squaredWeights[index] >= -0.5;
  }
  Arrayd _squaredWeights;
};

class WeighingStrategy {
 public:
  virtual void apply(double constraintWeight,
      Arrayd residuals, Weigher *dst) const = 0;
  virtual ~WeighingStrategy() {}

  typedef std::shared_ptr<WeighingStrategy> Ptr;
};

typedef Array<WeighingStrategy::Ptr> WeighingStrategies;

// Apply weights so that a subset of the rows in are treated as
// hard equality constraints
class ConstraintGroup : public WeighingStrategy {
 public:
  ConstraintGroup(Array<Spani> spans, int activeCount) :
    _spans(spans), _activeCount(activeCount), _minResidual(1.0e-9) {}

  void apply(double constraintWeight,
      Arrayd residuals, Weigher *dst) const;
 private:
  Array<Spani> _spans;
  int _activeCount;
  double _minResidual;
};


Eigen::VectorXd solve(
    const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
    WeighingStrategies strategies,
    Settings settings);


}
}

#endif /* SERVER_MATH_SPARSITYCONSTRAINED_H_ */
