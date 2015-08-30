/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SparsityConstrained.h>
#include <cmath>
#include <server/common/math.h>
#include <server/math/BandMat.h>
#include <server/common/LineKM.h>
#include <Eigen/SparseQR>
#include <Eigen/SparseCholesky>
#include <server/common/string.h>
#include <server/common/ScopedLog.h>

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
Arrayd distributeWeights(Arrayd residuals, double avgWeight) {
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
    Array<Residual> residuals, Arrayd weights) {

  DiagMat W(aRows);
  W.setIdentity();
  auto &v = W.diagonal();
  for (int i = 0; i < residuals.size(); i++) {
    auto span = residuals[i].span;
    auto w = weights[i];
    for (auto i: span) {
      v(i) = w;
    }
  }
  return W;
}



DiagMat makeWeightMatrix(
    int aRows,
    Array<Spani> allConstraintGroups, int activeCount,
  const Eigen::VectorXd &residualVector, double avgWeight, double minResidual) {

  Array<Residual> residualsPerConstraint = buildResidualsPerConstraint(allConstraintGroups,
    residualVector);

  auto thresholdedResiduals = threshold(residualsPerConstraint, activeCount, minResidual);
  Arrayd weights = distributeWeights(
      thresholdedResiduals,
      avgWeight);

  if (weights.empty()) {
    return DiagMat();
  }
  assert(weights.size() == residualsPerConstraint.size());
  return makeWeightMatrixSub(aRows, residualsPerConstraint, weights);
}



// Because directly computing Eigen::VectorXd result = A*X
// doesn't seem to work in Eigen :-(
Eigen::VectorXd product(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &X) {
  assert(A.cols() == X.size());
  Eigen::VectorXd Y = Eigen::VectorXd::Zero(A.rows());
  for (int k = 0; k < A.outerSize(); ++k) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(A, k); it; ++it) {
      Y(it.row()) += it.value()*X(it.col());
    }
  }
  return Y;
}

//typedef Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int> > Decomp;
typedef Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > Decomp;


Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
  Array<Spani> allConstraintGroups, int activeCount, Settings settings) {
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
    auto W = makeWeightMatrix(A.rows(), allConstraintGroups, activeCount, residuals,
        constraintWeight, settings.minResidual);
    if (W.size() == 0) {
      return X;
    }
    Eigen::SparseMatrix<double> WA = W*A;
    Eigen::VectorXd WB = W*B;

    //X = Decomp(WA).solve(WB);
    X = Decomp(WA.transpose()*WA).solve(WA.transpose()*WB);

    residuals = product(A, X) - B;
  }
  return X;
}


}
}
