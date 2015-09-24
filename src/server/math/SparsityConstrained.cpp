/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SparsityConstrained.h>
#include <cmath>
#include <server/common/math.h>
#include <server/math/BandMat.h>
#include <server/common/LineKM.h>
#include <Eigen/SparseCholesky>
#include <server/common/string.h>
#include <server/common/ScopedLog.h>
#include <server/common/ArrayIO.h>

namespace sail {
namespace SparsityConstrained {

typedef Eigen::Triplet<double> Triplet;

struct Residual {
 Spani span;
 double value;

 bool operator< (const Residual &other) const {
   return value < other.value;
 }
};

double calcResidualForSpan(Spani span, const Eigen::VectorXd &residualVector) {
  if (span.width() == 1) {
    return std::abs(residualVector(span.minv()));
  } else {
    double sum = 0.0;
    for (auto i: span) {
      sum += sqr(residualVector(i));
    }
    return (sum <= 0? 0 : sqrt(sum));
  }
}

Array<Residual> buildResidualsPerConstraint(Array<Spani> allConstraintGroups,
  const Eigen::VectorXd &residualVector) {
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

// Minimize w.r.t. W: |diag(W)*sqrt(residuals)|^2 subject to average(W) = avgWeight.
// All residuals must be positive.
Arrayd distributeWeights2(Arrayd residuals, double avgWeight) {
  assert(allPositive(residuals));
  int n = residuals.size();
  int m = n - 1;
  BandMat<double> AtA(m, m, 1, 1);
  MDArray2d AtB(m, 1);
  for (int i = 0; i < m; i++) {
    auto a = residuals[i];
    auto b = residuals[i+1];
    AtA(i, i) = a + b;
    AtB(i, 0) = -avgWeight*(a - b);
  }
  for (int i = 0; i < m-1; i++) {
    int next = i+1;
    auto r = residuals[next];
    AtA(i, next) = -r;
    AtA(next, i) = -r;
  }
  if (bandMatGaussElimDestructive(&AtA, &AtB)) {
    auto lambda = AtB.getStorage();
    Arrayd weights(n);
    weights.first() = lambda.first() + avgWeight;
    for (int i = 0; i < m-1; i++) {
      weights[i+1] = -lambda[i] + lambda[i+1] + avgWeight;
    }
    weights.last() = -lambda.last() + avgWeight;
    return weights;
  }
  return Arrayd();
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

typedef Eigen::DiagonalMatrix<double, Eigen::Dynamic, Eigen::Dynamic> DiagMat;

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



DiagMat makeWeightMatrix(
    int aRows,
    Array<ConstraintGroup> cstGroups,
  const Eigen::VectorXd &residualVector, double avgWeight, double minResidual) {

  int groupCount = cstGroups.size();

  Array<Array<Residual> > residualsPerGroup(groupCount);
  Array<Arrayd> weightsPerGroup(groupCount);

  for (int i = 0; i < groupCount; i++) {
    auto group = cstGroups[i];
    Array<Residual> residualsPerConstraint = buildResidualsPerConstraint(group.spans,
      residualVector);

    auto thresholdedResiduals = threshold(residualsPerConstraint, group.activeCount, minResidual);
    Arrayd weights = distributeWeights(
        thresholdedResiduals,
        avgWeight);

    if (weights.empty()) {
      return DiagMat();
    }
    assert(weights.size() == residualsPerConstraint.size());
    weightsPerGroup[i] = weights;
    residualsPerGroup[i] = residualsPerConstraint;
  }
  return makeWeightMatrixSub(aRows, residualsPerGroup, weightsPerGroup);
}

Eigen::VectorXd product(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &X) {
  Eigen::VectorXd Y = Eigen::VectorXd::Zero(A.rows());
  Y += A*X;
  return Y;
}

Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
    Array<ConstraintGroup> cstGroups, Settings settings) {
  ENTERSCOPE("SparsityConstrained::Solve");
  int rows = A.rows();
  assert(rows == B.rows());
  Eigen::VectorXd residuals = Eigen::VectorXd::Constant(rows, 1.0);
  Eigen::VectorXd X;
  LineKM logWeights(0, settings.iters-1, log(settings.initialWeight), log(settings.finalWeight));
  for (int i = 0; i < settings.iters; i++) {
    double constraintWeight = exp(logWeights(i));
    SCOPEDMESSAGE(INFO, stringFormat("  Iteration %d/%d with weight %.3g",
        i+1, settings.iters, constraintWeight));
    auto W = makeWeightMatrix(A.rows(), cstGroups, residuals,
        constraintWeight, settings.minResidual);
    if (W.size() == 0) {
      return X;
    }
    Eigen::SparseMatrix<double> WA = W*A;
    Eigen::VectorXd WB = W*B;

    X = Decomp(WA.transpose()*WA).solve(WA.transpose()*WB);

    residuals = product(A, X) - B;
  }
  return X;
}


}
}
