/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/irls.h>
#include <cmath>
#include <server/common/math.h>
#include <server/math/BandMat.h>
#include <server/common/LineKM.h>
#include <Eigen/SparseCholesky>
#include <server/common/string.h>
#include <server/common/ScopedLog.h>
#include <server/common/ArrayIO.h>

namespace sail {
namespace irls {

typedef Eigen::Triplet<double> Triplet;

struct Residual {
 Spani span;
 double value;

 bool operator< (const Residual &other) const {
   return value < other.value;
 }
};

double calcResidualForSpan(Spani span, const Arrayd &residualVector) {
  if (span.width() == 1) {
    return std::abs(residualVector[span.minv()]);
  } else {
    double sum = 0.0;
    for (auto i: span) {
      sum += sqr(residualVector[i]);
    }
    return (sum <= 0? 0 : sqrt(sum));
  }
}

Array<Residual> buildResidualsPerConstraint(Array<Spani> allConstraintGroups,
  const Arrayd &residualVector) {
  int n = allConstraintGroups.size();
  Array<Residual> dst(n);
  for (int i = 0; i < n; i++) {
    auto span = allConstraintGroups[i];
    dst[i] = Residual{span, calcResidualForSpan(span, residualVector)};
  }
  std::sort(dst.begin(), dst.end());
  return dst;
}


bool allPositive(Arrayd X) {
  return X.all([](double x) {return x > 0;});
}


double getSqrtResidual(const Arrayd &residuals, int index) {
  double marg = 1.0e-12; // <- for well-posedness.
  return sqrt(residuals[index] + marg);
}

typedef Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > Decomp;


Arrayd distributeWeights(Arrayd residuals, double avgWeight) {
  int n = residuals.size();
  int paramCount = n-1;
  int elemCount = 2*paramCount;
  Array<Triplet> triplets(elemCount);
  for (int i = 0; i < paramCount; i++) {
    int offset = 2*i;
    double r0 = getSqrtResidual(residuals, i);
    double r1 = getSqrtResidual(residuals, i+1);
    triplets[offset + 0] = Triplet(i+0, i, r0);
    triplets[offset + 1] = Triplet(i+1, i, -r1);
  }
  Eigen::VectorXd B(n);
  for (int i = 0; i < n; i++) {
    B[i] = -getSqrtResidual(residuals, i)*avgWeight;
  }
  Eigen::SparseMatrix<double> A(n, paramCount);
  A.setFromTriplets(triplets.begin(), triplets.end());
  Decomp decomp(A.transpose()*A);
  Eigen::VectorXd result = decomp.solve(A.transpose()*B);
  Arrayd weights = Arrayd::fill(n, avgWeight);
  for (int i = 0; i < paramCount; i++) {
    weights[i] += result[i];
    weights[i+1] -= result[i];
  }
  return weights;
}

Arrayd threshold(Array<Residual> residuals, int activeCount, double minResidual) {
  int n = residuals.size();
  Arrayd Y(n);
  Y.sliceTo(activeCount).setTo(std::max(minResidual, residuals[activeCount-1].value));
  for (int i = activeCount; i < n; i++) {
    Y[i] = std::max(minResidual, residuals[i].value);
  }
  return Y;
}

int countCoefs(Array<Residual> residuals) {
  int counter = 0;
  for (auto r: residuals) {
    counter += r.span.width();
  }
  return counter;
}


DiagMat makeWeightMatrixSub(int aRows,
    Array<Array<Residual> > residualsPerGroup, Array<Arrayd> weightsPerGroup) {

  int n = residualsPerGroup.size();

  DiagMat W(aRows);
  W.setIdentity();
  auto &v = W.diagonal();
  for (int i = 0; i < n; i++) {
    auto residuals = residualsPerGroup[i];
    auto weights = weightsPerGroup[i];
    for (int i = 0; i < residuals.size(); i++) {
      auto span = residuals[i].span;
      auto w = weights[i];
      for (auto i: span) {
        v(i) = w;
      }
    }
  }
  return W;
}

QuadCompiler::WeightsAndOffset QuadCompiler::makeWeightsAndOffset() const {
  int n = _quads.size();
  DiagMat W(n);
  Eigen::VectorXd offsets(n);
  W.setIdentity();
  auto &v = W.diagonal();
  for (int i = 0; i < n; i++) {
    const MajQuad &q = _quads[i];
    if (q.defined()) {
      auto line = q.factor();
      v(i) = line.getK();
      offsets(i) = line.getM();
    } else {
      v(i) = 1.0;
      offsets(i) = 0.0;
    }
  }
  return WeightsAndOffset{W, offsets};
}

void ConstraintGroup::apply(double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) const {
  Array<Residual> residualsPerConstraint = buildResidualsPerConstraint(_spans,
    residuals);

  auto thresholdedResiduals = threshold(residualsPerConstraint, _activeCount, _minResidual);
  Arrayd weights = distributeWeights(
      thresholdedResiduals,
      constraintWeight);

  int n = weights.size();
  assert(n == _spans.size());
  assert(n == residualsPerConstraint.size());
  for (int i = 0; i < n; i++) {
    double w = weights[i];
    const Spani &span = residualsPerConstraint[i].span;
    for (auto j : span) {
      dst->setWeight(j, w);
    }
  }
}

void InequalityConstraint::apply(double constraintWeight,
  const Arrayd &residuals, QuadCompiler *dst) const {
  assert(_index != -1);
  double r = residuals[_index];
  auto absQuad = MajQuad::majorizeAbs(r, LB);
  auto q = constraintWeight*(absQuad + MajQuad::linear(-1.0))
      + MajQuad::linear(_reg);
  auto line = q.factor();
  dst->addQuad(_index, q);
}

WeightingStrategy::Ptr InequalityConstraint::make(Arrayi inds, double reg) {
  return WeightingStrategy::Ptr(
      new WeightingStrategyArray<InequalityConstraint>(
          inds.map<InequalityConstraint>([&](int index) {
    return InequalityConstraint(index, reg);
  })));
}

double calcNorm(Spani span, const Arrayd &residuals) {
  double squaredNorm = 0.0;
  for (auto i : span) {
    squaredNorm += sqr(residuals[i]);
  }
  return (squaredNorm < 0? 0 : sqrt(squaredNorm));
}

void BoundedNormConstraint::apply(double constraintWeight,
    const Arrayd &residuals, QuadCompiler *dst) const {
  double norm = calcNorm(_span, residuals);
  auto shifted = norm - _bound;
  auto qShifted = MajQuad::majorizeAbs(shifted, LB) + MajQuad::linear(1.0);
  auto opt_ = qShifted.optimimum();
  assert(opt_ <= 0);
  double optimum = _bound + opt_;
  if (optimum >= 0) {
    double f = optimum/(norm + LB);
    for (auto i : _span) {
      dst->addQuad(i, constraintWeight*MajQuad::fit(f*residuals[i]));
    }
  } else {
    auto maj = 2.0*constraintWeight*MajQuad::majorizeAbs(norm, LB);
    for (auto i : _span) {
      dst->addQuad(i, maj);
    }
  }
}

// Make multiple constraints with the same bound
WeightingStrategy::Ptr BoundedNormConstraint::make(Array<Spani> spans, double bound) {
  return WeightingStrategy::Ptr(
      new WeightingStrategyArray<BoundedNormConstraint>(
          spans.map<BoundedNormConstraint>([&](Spani span) {
            return BoundedNormConstraint(span, bound);
          })));
}


void ConstantNormConstraint::apply(double constraintWeight,
    const Arrayd &residuals, QuadCompiler *dst) const {
  double norm = calcNorm(_span, residuals);
  double f = _length/(norm + LB);
  for (auto i : _span) {
    dst->addQuad(i, constraintWeight*MajQuad::fit(f*residuals[i]));
  }
}

// Make multiple constraints with the same length.
WeightingStrategy::Ptr ConstantNormConstraint::make(Array<Spani> spans, double length) {
  return WeightingStrategy::Ptr(
        new WeightingStrategyArray<ConstantNormConstraint>(
            spans.map<ConstantNormConstraint>([&](Spani span) {
              return ConstantNormConstraint(span, length);
            })));
}


Eigen::VectorXd product(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &X) {
  Eigen::VectorXd Y = Eigen::VectorXd::Zero(A.rows());
  Y += A*X;
  return Y;
}

Arrayd toArray(Eigen::VectorXd &v) {
  return Arrayd(v.size(), v.data());
}

Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A,
    const Eigen::VectorXd &B,
    Array<std::shared_ptr<WeightingStrategy> > strategies,
    Settings settings) {
  ENTERSCOPE("SparsityConstrained::Solve");
  int rows = A.rows();
  assert(rows == B.rows());
  Eigen::VectorXd residuals = Eigen::VectorXd::Constant(rows, 1.0);
  Eigen::VectorXd X;


  LineKM logWeights(0, settings.iters-1,
      log(settings.initialWeight), log(settings.finalWeight));

  for (int i = 0; i < settings.iters; i++) {
    double constraintWeight = exp(logWeights(i));

    SCOPEDMESSAGE(INFO, stringFormat("  Iteration %d/%d with weight %.3g",
        i+1, settings.iters, constraintWeight));

    QuadCompiler weigher(residuals.size());
    auto residualArray = toArray(residuals);
    for (auto strategy: strategies) {
      CHECK(bool(strategy));
      strategy->apply(constraintWeight, residualArray, &weigher);
    }
    auto wk = weigher.makeWeightsAndOffset();

    Eigen::SparseMatrix<double> WA = wk.weights*A;
    Eigen::VectorXd WB = wk.weights*B - wk.offset;
    X = Decomp(WA.transpose()*WA).solve(WA.transpose()*WB);

    residuals = product(A, X) - B;
  }
  return X;
}


}
}
