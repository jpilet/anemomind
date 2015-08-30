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

/*
 * Solves a least squares problem with sparsity constraints.
 *
 * The matrix A has as many rows as the dimension of vector B.
 * So every row of A has a corresponding element in B.
 *
 * Every constraint group is a set of row spans of A and B that should be enforced
 * as equality constraints. From every ConstraintGroup, activeCount such spans
 * among all spans will be enforced and the remaining ones will be passive.
 *
 * Let A. and B. be the matrices formed by rows from A and B that are not part of
 * any constraint. This algorithm will minimize |A. X - B.|^2 subject to choosing
 * activeCount constraints from every constraint group.
 */
Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
  Array<ConstraintGroup> cstGroups, Settings settings);

}
}

#endif /* SERVER_MATH_SPARSITYCONSTRAINED_H_ */
