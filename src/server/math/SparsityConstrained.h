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

struct Settings {
 int iters = 30;
 double initialWeight = 0.1;
 double finalWeight = 10000;
 double minResidual = 1.0e-9;
};

// Minimize |AX - B|^2 subject to freely picking exactly
// activeCount constraint groups among allConstraintGroups.
// All rows in A and B that belong to passive constraint groups
// are weighted by 0.
//
// A constraint group is a span of the residuals in R = AX - B
// that should be exactly 0.
//
// Application: Solving least squares problems, subject
// to sparsity constraints. We can for instance use this
// to fit exactly M line segments to noisy data in least squares sense.
Eigen::VectorXd solve(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &B,
  Array<Spani> allConstraintGroups, int activeCount, Settings settings);

}
}

#endif /* SERVER_MATH_SPARSITYCONSTRAINED_H_ */
