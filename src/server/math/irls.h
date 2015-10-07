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
#include <server/math/Majorize.h>

namespace sail {
namespace irls {

/*
 * Performs Iteratively Reweighted Least Squares. This is the name of the practice
 * of solving least squares problems of type |W*(A*X - B)|^2, where W is a diagonal matrix
 * whose non-zero entries are updated for every iteration. Actually, we tweaked it a bit
 * by also introducing a vector K that varies in each iteration, so that we minimize
 * |W*(A*X - B) + K|^2 which lets us do even cooler things.
 *
 * Classes that inherit from WeightingStrategy specify how the weights and entries
 * in K are computed.
 *
 * By the way, here is the difference between the verbs 'weigh' and 'weight' explained:
 * http://forum.wordreference.com/threads/weighing-versus-weighting.88753/
 */


// Minimize w.r.t. W: |diag(W)*sqrt(residuals)|^2 subject to average(W) = avgWeight.
// All residuals must be positive. Used by the class ConstraintGroup
Arrayd distributeWeights(Arrayd residuals, double avgWeight);

struct Settings {
 // Some weighting strategies implement hard constraints. We approximate those
 // constraints by applying a weight that grows towards infinity. It seems a bit
 // dirty but works pretty well.

 // More iterations will likely lead to more accurate results
 // and better convergence.
 int iters = 30;

 // You might want to tune this depending on the
 // objective function, but try without tuning first.
 double initialWeight = 0.1;

 // I guess this is a suitable final weight in most cases.
 // Not much need to tune it.
 double finalWeight = 10000;
};

typedef Eigen::DiagonalMatrix<double, Eigen::Dynamic, Eigen::Dynamic> DiagMat;

// Used to accumulate the weights and right-hand-sides in every iteration.
class QuadCompiler {
 public:
  QuadCompiler(int dim) : _quads(dim) {}

  // The main method used by the weighting strategies.
  void addQuad(int index, const MajQuad &q) {
    MajQuad &dst = _quads[index];
    if (dst.defined()) {
      dst = dst + q;
    } else {
      dst = q;
    }
  }

  // Special case, when there is only a weight and
  // no offset.
  void setWeight(int index, double weight) {
    addQuad(index, MajQuad(sqr(weight), 0.0));
  }

  // Output of the method below.
  struct WeightsAndOffset {
    DiagMat weights;
    Eigen::VectorXd offset;
  };

  // Once the iteration is done,
  // this method will provide the matrices
  // needed to tweak the problem.
  WeightsAndOffset makeWeightsAndOffset() const;
 private:
  Array<MajQuad> _quads;
};

// A WeightingStrategy defines how the least-squares problem should be tweaked in
// each iteration to solve the problem at hand.
class WeightingStrategy {
 public:
  static constexpr double LB = 1.0e-9;

  virtual void apply(double constraintWeight,
      const Arrayd &residuals, QuadCompiler *dst) const = 0;
  virtual ~WeightingStrategy() {}

  typedef std::shared_ptr<WeightingStrategy> Ptr;
};

typedef Array<WeightingStrategy::Ptr> WeightingStrategies;

// Groups several constraints of the same kind as a contiguous
// array of values, in the hope of benefits in terms of cache locality.
template <typename T>
class WeightingStrategyArray : public WeightingStrategy {
 public:
  WeightingStrategyArray(Array<T> strategies) : _strategies(strategies) {}

  void apply(double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) const {
    for (const T &s: _strategies) {
      s.apply(constraintWeight, residuals, dst);
    }
  }

  static WeightingStrategy::Ptr make(Array<T> strategies) {
    return WeightingStrategy::Ptr(new WeightingStrategyArray<T>(strategies));
  }
 private:
  Array<T> _strategies;
};

// A subset of the rows will be treated as hard constraints.
// Each span is a span of rows that should simultaneously be either enforced or not.
// activeCount determines how many spans should be enforced. This can for instance be used
// to force a variable to be binary. We can create a constraint group with the constraints
//  (i)  x = 0
//  (ii) x = 1
// and choose activeCount=1. Then either x = 0 or x = 1 must be true.
class ConstraintGroup : public WeightingStrategy {
 public:
  ConstraintGroup(Array<Spani> spans, int activeCount) :
    _spans(spans), _activeCount(activeCount), _minResidual(1.0e-9) {}

  void apply(double constraintWeight,
      const Arrayd &residuals, QuadCompiler *dst) const;
 private:
  Array<Spani> _spans;
  int _activeCount;
  double _minResidual;
};

// Inequality constraint on the form A*X - B >= 0
// 'reg' adds an extra cost f(X) = reg*(A*X - B). It is
// convient to couple such a cost with the inequality constraint.
class InequalityConstraint : public WeightingStrategy {
 public:
  InequalityConstraint() : _index(-1), _reg(NAN) {}
  InequalityConstraint(int index, double reg) : _index(index), _reg(reg) {}

  void apply(double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) const;

  // Make multiple constraints with the same regularization
  static WeightingStrategy::Ptr make(Arrayi inds, double reg);
 private:
  double _reg;
  int _index;
};


// Constraint on the form |A*X - B| <= bound
class BoundedNormConstraint : public WeightingStrategy {
 public:
  BoundedNormConstraint() : _bound(NAN) {}
  BoundedNormConstraint(Spani span, double bound) : _span(span), _bound(bound) {}

  void apply(double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) const;

  // Make multiple constraints with the same bound
  static WeightingStrategy::Ptr make(Array<Spani> spans, double bound);
 private:
  Spani _span;
  double _bound;
};

// Constraint on the form |A*X - B| = length
// Suitable for constraining the lengths of edges in a mesh.
class ConstantNormConstraint : public WeightingStrategy {
 public:
  ConstantNormConstraint() : _length(NAN) {}
  ConstantNormConstraint(Spani span, double length) : _span(span), _length(length) {}

  void apply(double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) const;

  // Make multiple constraints with the same length.
  static WeightingStrategy::Ptr make(Array<Spani> spans, double length);
 private:
  Spani _span;
  double _length;
};

Eigen::VectorXd solve(
    const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
    WeightingStrategies strategies,
    Settings settings);


}
}

#endif /* SERVER_MATH_SPARSITYCONSTRAINED_H_ */
