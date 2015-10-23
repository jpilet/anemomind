/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_POWERMETHO_H_
#define SERVER_MATH_POWERMETHO_H_

#include <Eigen/Core>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {
namespace PowerMethod {

/*
 * Compute the minimum and maximum eigenvectors of a positive
 * semi-definite matrix, using the powermethod. If the matrix
 * is A, all we need to know is a function f(X) = A*X
 * that multiplies a vector with that matrix. That is,
 * we don't need to know the matrix A explicitly. For instance,
 * if A = P*P', with P being a huge column vector, it is
 * going to be expensive to evaluate P*P' explicitly. Instead,
 * we evaluate f(X) = A*X = P*(P'*X) instead of A*X = (P*P')*X
 *
 * A typical use case for this
 * is the following optimization problem:
 *
 * Minimize w.r.t. X: |A*X|^2
 * subject to |X| = 1
 *
 * The solution to this problem is the minimum eigenvector
 * of A'*A.
 * This algorithm lets us obtain that vector of A'*A
 * in an computationally inexpensive way.
 */

// Represents the multiplication with some matrix A:
// Y = A*X
typedef std::function<Eigen::VectorXd(Eigen::VectorXd)> MatMul;

double sinAngle(const Eigen::VectorXd &a, const Eigen::VectorXd &b);

struct Settings {
 // Stop when either maxIters are reached, or
 // the angle between the vector from one iteration
 // to the next is less than tol.
 int maxIters = 300;
 Angle<double> tol = Angle<double>::radians(1.0e-4);
};

struct Results {
  Eigen::VectorXd X;
  int iters;
};

// Compute the eigenvector with the greatest eigenvalue.
// The norm of the vector is the actual eigenvalue.
Results computeMax(MatMul A, const Eigen::VectorXd &X, const Settings &s);

// Compute the eigenvector with the smallest eigenvalue.
// The norm of the vector is the actual eigenvalue.
Results computeMin(MatMul A, const Eigen::VectorXd &X, const Settings &s);



}
}

#endif /* SERVER_MATH_POWERMETHO_H_ */
