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
#include <server/common/math.h>
#include <server/math/Majorize.h>
#include <server/math/QuadForm.h>

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

  virtual void apply(
      double constraintWeight,
      const Arrayd &residuals, QuadCompiler *dst) = 0;

  virtual void initialize(const Settings &s, QuadCompiler *dst) = 0;

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

  void apply(
      double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) {
    for (T &s: _strategies) {
      s.apply(constraintWeight, residuals, dst);
    }
  }

  void initialize(const Settings &s, QuadCompiler *dst) {
    for (T &x: _strategies) {
      x.initialize(s, dst);
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
  ConstraintGroup() : _activeCount(0), _minResidual(NAN) {}

  ConstraintGroup(Array<Spani> spans, int activeCount) :
    _spans(spans), _activeCount(activeCount), _minResidual(1.0e-9) {}

  static WeightingStrategy::Ptr make(Array<Spani> spans, int activeCount);

  void apply(
        double constraintWeight,
        const Arrayd &residuals, QuadCompiler *dst);

  void initialize(const Settings &s, QuadCompiler *dst);
 private:
  Array<Spani> _spans;
  int _activeCount;
  double _minResidual;
};

// Like ConstraintGroup, but for the special case
// of two spans among which one should be active.
class BinaryConstraintGroup : public WeightingStrategy {
 public:
  BinaryConstraintGroup() {}
  BinaryConstraintGroup(const Spani &a, const Spani &b) : _a(a), _b(b) {}

  void apply(double constraintWeight,
      const Arrayd &residuals, QuadCompiler *dst);

  void initialize(const Settings &s, QuadCompiler *dst);
 private:
  Spani _a, _b;
};

class NonNegativeConstraint : public WeightingStrategy {
 public:
  NonNegativeConstraint() : _lastWeight(0), _index(-1) {}
  NonNegativeConstraint(int index) :  _lastWeight(0), _index(index) {}
  void apply(
    double constraintWeight,
    const Arrayd &residuals, QuadCompiler *dst) {
    // Modification of the penalty method:
    // See section A1:
    // https://www.me.utexas.edu/~jensen/ORMM/supplements/units/nlp_methods/const_opt.pdf
    auto r = residuals[_index];
    if (r < 0) {
      _lastWeight = constraintWeight;
      dst->setWeight(_index, constraintWeight);
    } else {
      // This is our tweak: Even for a feasible solution,
      // apply some damping to avoid oscillations.
      // Decrease this damping (by how much?) for every
      // iteration that is feasible.
      dst->addQuad(_index, sqr(_lastWeight)*MajQuad::fit(r));
      _lastWeight *= 0.5;
    }

  }

  void initialize(const Settings &s, QuadCompiler *dst) {
    dst->setWeight(_index, s.initialWeight);
  }

  static WeightingStrategy::Ptr make(int index);
  static WeightingStrategy::Ptr make(Arrayi inds);
 private:
  double _lastWeight;
  int _index;
};

class Constant : public WeightingStrategy {
 public:
  Constant() : _index(-1) {}
  Constant(int index, MajQuad quad) : _index(index), _quad(quad) {}

  void apply(
    double constraintWeight,
    const Arrayd &residuals, QuadCompiler *dst) {
    dst->addQuad(_index, _quad);
  }

  void initialize(const Settings &s, QuadCompiler *dst) {
    dst->setWeight(_index, s.finalWeight);
  }

  static WeightingStrategy::Ptr make(Arrayi inds, MajQuad quad);
  static WeightingStrategy::Ptr make(int index, MajQuad quad);
 private:
  int _index;
  MajQuad _quad;
};

struct Results {
 Eigen::VectorXd X;
 Eigen::VectorXd residuals;
};

/********************************************************************
 *
 * General sparse problems
 *
 */
Results solve(const Eigen::SparseMatrix<double> &A,
    const Eigen::VectorXd &B,
    Array<std::shared_ptr<WeightingStrategy> > strategies,
    Settings settings);






/*********************************************************************
 *
 * Problems that result in a band matrix
 *
 */
/*class DenseBlock {
public:
  virtual int lhsCols() const = 0;
  virtual int rhsCols() const = 0;
  virtual int requiredRows() const = 0;
  virtual int requiredCols() const = 0;

  virtual void accumulateWeighted(const Eigen::VectorXd &weights,
      BandMatrix<double> *AtA, MDArray2d *AtB) const = 0;

  int minDiagWidth() const;

  virtual void eval(const Eigen::MatrixXd &X,
      Eigen::VectorXd *residuals) const = 0;

  typedef std::shared_ptr<DenseBlock> Ptr;
  virtual ~DenseBlock() {}
};


Eigen::Map<Eigen::MatrixXd,
  Eigen::Unaligned,
    Eigen::Stride<Eigen::Dynamic, 1> > bandMatrixView(
        BandMatrix<double> *src, int N, int offset);

Eigen::Map<Eigen::MatrixXd,
    Eigen::Unaligned, Eigen::OuterStride<> > arrayView(
        MDArray2d *X, int rows, int cols, int rowOffsest);*/

}
}

#endif /* SERVER_MATH_SPARSITYCONSTRAINED_H_ */
