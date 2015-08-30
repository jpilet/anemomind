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

namespace sail {
namespace SparsityConstrained {


// Minimize w.r.t. W: |diag(W)*sqrt(residuals)|^2 subject to average(W) = avgWeight.
// All residuals must be positive.
Arrayd distributeWeights(Arrayd residuals, double avgWeight);

struct Settings {
 int iters = 30;
 double initialWeight = 0.1;
 double finalWeight = 10000;
 double minResidual = 1.0e-9;
};

struct ConstraintGroup {
 Array<Spani> spans;
 int activeCount;
};

Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
  Array<ConstraintGroup> cstGroups, Settings settings);

}
}

#endif /* SERVER_MATH_SPARSITYCONSTRAINED_H_ */
