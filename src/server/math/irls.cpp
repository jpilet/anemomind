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
#include <server/common/Functional.h>

namespace sail {
namespace irls {

typedef Eigen::Triplet<double> Triplet;

bool isSane(int n, const double *X) {
  int counter = 0;
  int maxWarnings = 3;
  for (int i = 0; i < n; i++) {
    if (!std::isfinite(X[i])) {
      if (counter < maxWarnings) {
        LOG(WARNING) << "Element " << i << " is bad.";
      }
      counter++;
    }
  }
  if (counter > maxWarnings) {
    LOG(WARNING) << " ...and " << counter - maxWarnings << " other elements.";
  }
  return counter == 0;
}

bool isSane(Arrayd X) {
  return isSane(X.size(), X.ptr());
}

bool isSane(const Eigen::VectorXd &X) {
  return isSane(X.size(), X.data());
}

Arrayd toArray(Eigen::VectorXd &v) {
  return Arrayd(v.size(), v.data());
}


struct Residual {
 Spani span;
 int index;
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
    dst[i] = Residual{span, i, calcResidualForSpan(span, residualVector)};
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
  assert(isSane(v));
  assert(isSane(offsets));
  return WeightsAndOffset{W, offsets};
}

Arrayi ConstraintGroup::computeActiveSpans(Eigen::VectorXd residuals) {
  Arrayd residualArray = toArray(residuals);
  Array<Residual> residualsPerConstraint = buildResidualsPerConstraint(_spans,
    residualArray);
  std::cout << EXPR_AND_VAL_AS_STRING(_activeCount) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(_spans.size()) << std::endl;
  return toArray(map([](const Residual &r) {
    return r.index;
  }, residualsPerConstraint.sliceTo(_activeCount)));
}

void ConstraintGroup::apply(
    double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) {
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

void BinaryConstraintGroup::apply(double constraintWeight,
    const Arrayd &residuals, QuadCompiler *dst) {
  auto totalCstWeight = 2.0*constraintWeight;
  auto ra = calcResidualForSpan(_a, residuals);
  auto rb = calcResidualForSpan(_b, residuals);
  auto aw = toFinite(rb/(ra + rb), 0.5);
  auto bw = 1.0 - aw;
  auto awc = totalCstWeight*aw;
  auto bwc = totalCstWeight*bw;
  for (auto i : _a) {
    dst->setWeight(i, awc);
  }
  for (auto i : _b) {
    dst->setWeight(i, bwc);
  }
}

int BinaryConstraintGroup::getBestFitIndex(Eigen::VectorXd v) const {
  return getBestFitIndex(Arrayd(v.size(), v.data()));
}

int BinaryConstraintGroup::getBestFitIndex(const Arrayd &residuals) const {
  return (calcResidualForSpan(_a, residuals)
      < calcResidualForSpan(_b, residuals)? 0 : 1);
}

WeightingStrategy::Ptr ConstraintGroup::make(Array<Spani> spans, int activeCount) {
  return WeightingStrategy::Ptr(new ConstraintGroup(spans, activeCount));
}

WeightingStrategy::Ptr Constant::make(Arrayi inds, MajQuad quad) {
  return WeightingStrategyArray<Constant>::make(toArray(map(
      [=](int index) {
    return Constant(index, quad);
  }, inds)));
}

WeightingStrategy::Ptr NonNegativeConstraint::make(int index) {
  return WeightingStrategy::Ptr(new NonNegativeConstraint(index));
}

WeightingStrategy::Ptr NonNegativeConstraint::make(Arrayi inds) {
  return WeightingStrategyArray<NonNegativeConstraint>::make(toArray(map(
      [=](int index) {
    return NonNegativeConstraint(index);
  }, inds)));
}

WeightingStrategy::Ptr Constant::make(int index, MajQuad quad) {
  return WeightingStrategy::Ptr(new Constant(index, quad));
}

FitNorm::FitNorm(Spani Xspan, int aRow, int n, bool constraint) :
    _Xspan(Xspan), _aRow(aRow), _n(n), _constraint(constraint) {}

void FitNorm::apply(
  double constraintWeight,
  const Arrayd &residuals, QuadCompiler *dst) {
  double currentNorm = calcResidualForSpan(_Xspan, residuals);
  double targetNorm = residuals[_aRow];
  double error = std::abs(currentNorm - targetNorm);
  double fError = std::pow(error, _n);
  double deriv = _n*fError/(error + 1.0e-12);
  double c = (_constraint? sqr(constraintWeight) : 1.0);
  auto w = 2.0*c*MajQuad::majorize(thresholdCloseTo0(error, 1.0e-9),
      fError, deriv, 0.0).a;
  double meanNorm = 0.5*(currentNorm + targetNorm);
  double factor = meanNorm/currentNorm;
  for (auto i: _Xspan) {
    dst->addQuad(i, w*MajQuad::fit(factor*residuals[i]));
  }
  dst->addQuad(_aRow, w*MajQuad::fit(meanNorm));
}


Eigen::VectorXd product(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &X) {
  Eigen::VectorXd Y = Eigen::VectorXd::Zero(A.rows());
  Y += A*X;
  return Y;
}


Results solveFull(const Eigen::SparseMatrix<double> &A,
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

  LineKM weights(0, settings.iters, settings.initialWeight, settings.finalWeight);

  for (int i = 0; i < settings.iters; i++) {
    double constraintWeight = (settings.logWeighting?
        exp(logWeights(i)) : weights(i));

    SCOPEDMESSAGE(INFO, stringFormat("  Iteration %d/%d with weight %.3g",
        i+1, settings.iters, constraintWeight));

    QuadCompiler weighter(residuals.size());
    auto residualArray = toArray(residuals);
    assert(isSane(residualArray));
    for (auto strategy: strategies) {
      CHECK(bool(strategy));
      strategy->apply(constraintWeight, residualArray, &weighter);
    }
    auto wk = weighter.makeWeightsAndOffset();

    Eigen::SparseMatrix<double> WA = wk.weights*A;
    Eigen::VectorXd WB = wk.weights*B - wk.offset;

    assert(isSane(wk.offset));
    assert(isSane(B));
    assert(isSane(WB));

    X = Decomp(WA.transpose()*WA).solve(WA.transpose()*WB);

    assert(isSane(X));

    residuals = product(A, X) - B;

    assert(isSane(residuals));
  }
  return Results{X, residuals};
}

Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A,
    const Eigen::VectorXd &B,
    Array<std::shared_ptr<WeightingStrategy> > strategies,
    Settings settings) {
    return solveFull(A, B, strategies, settings).X;
}


}
}
