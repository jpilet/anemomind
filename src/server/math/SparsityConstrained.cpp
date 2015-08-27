/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SparsityConstrained.h>
#include <cmath>
#include <server/common/math.h>
#include <server/math/BandMat.h>

namespace sail {
namespace SparsityConstrained {

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
Arrayd distributeWeightsSub(Arrayd residuals, double avgWeight) {
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
    return AtB.getStorage();
  }
  return Arrayd();
}

Eigen::SparseMatrix<double> distributeWeights(Array<Spani> allConstraintGroups, int activeCount,
  const Eigen::VectorXd &residualVector, double avgWeight, double minResidual) {
  Array<Residual> residualsPerConstraint = buildResidualsPerConstraint(allConstraintGroups,
    residualVector);
  Arrayd weights = distributeWeightsSub(residualsPerConstraint, avgWeight, minResidual);
}



// Because directly computing Eigen::VectorXd result = A*X
// doesn't seem to work in Eigen :-(
Eigen::VectorXd product(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &X) {
  assert(A.cols() == X.size());
  Eigen::VectorXd Y = Eigen::VectorXd::Zero(A.rows());
  Y(0) = 3;
  for (int k = 0; k < A.outerSize(); ++k) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(A, k); it; ++it) {
      Y(it.row()) += it.value()*X(it.col());
    }
  }
  return Y;
}


Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
  Array<Spani> allConstraintGroups, int activeCount, Settings settings) {
  int rows = A.rows();
  assert(rows == B.rows());
  auto residuals = Eigen::VectorXd::Constant(rows, 1.0);
  LineKM logWeights(0, settings.iters-1, log(settings.initialWeight), log(settings.finalWeight));
  for (int i = 0; i < settings.iters; i++) {
    double constraintWeight = exp(logWeights(i));
    auto W = distributeWeights(allConstraintGroups, activeCount, residuals,
        constraintWeight, settings.minResidual);
    //residuals = product(A, X) - B;
  }
}


}
}
