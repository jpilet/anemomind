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
 * whose non-zero entries are updated for every iteration.
 *
 *
 * By the way, here is the difference between the verbs 'weigh' and 'weight' explained:
 * http://forum.wordreference.com/threads/weighing-versus-weighting.88753/
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

// Computes the right weights and right-hand-sides
class QuadCompiler {
 public:
  QuadCompiler(int dim) : _quads(dim) {}

  void addQuad(int index, const MajQuad &q) {
    MajQuad &dst = _quads[index];
    if (dst.defined()) {
      dst = dst + q;
    } else {
      dst = q;
    }
  }

  void setWeight(int index, double weight) {
    addQuad(index, MajQuad(sqr(weight), 0.0));
  }

  double calcWeight(int index) const {
    auto w = _quads[index];
    if (w.defined()) {
      auto f = w.factor();
      assert(std::abs(f.getM()) < 1.0e-6);
      return f.getK();
    }
    return 1.0;
  }

  struct WeightsAndOffset {
    DiagMat weights;
    Eigen::VectorXd offset;
  };

  WeightsAndOffset makeWeightAndOffset() const;
 private:
  Array<MajQuad> _quads;
};

// This is a strategy used to compute the weights of the rows.
class WeightingStrategy {
 public:
  virtual void apply(double constraintWeight,
      Arrayd residuals, QuadCompiler *dst) const = 0;
  virtual ~WeightingStrategy() {}

  typedef std::shared_ptr<WeightingStrategy> Ptr;
};

typedef Array<WeightingStrategy::Ptr> WeightingStrategies;

// Apply weights so that a subset of the rows in are treated as
// hard equality constraints
class ConstraintGroup : public WeightingStrategy {
 public:
  ConstraintGroup(Array<Spani> spans, int activeCount) :
    _spans(spans), _activeCount(activeCount), _minResidual(1.0e-9) {}

  void apply(double constraintWeight,
      Arrayd residuals, QuadCompiler *dst) const;
 private:
  Array<Spani> _spans;
  int _activeCount;
  double _minResidual;
};

class NonNegativeConstraints : public WeightingStrategy {
 public:
  NonNegativeConstraints(Arrayi inds, double reg) : _inds(inds), _reg(reg), _lb(1.0e-9) {}

  void apply(double constraintWeight, Arrayd residuals, QuadCompiler *dst) const;
 private:
  double _reg;
  Arrayi _inds;
  double _lb;
};

Eigen::VectorXd solve(
    const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
    WeightingStrategies strategies,
    Settings settings);


}
}

#endif /* SERVER_MATH_SPARSITYCONSTRAINED_H_ */
