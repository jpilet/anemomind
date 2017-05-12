/*
 * BandedIrls.h
 *
 *  Created on: 30 Mar 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_BANDEDIRLS_H_
#define SERVER_MATH_BANDEDIRLS_H_

#include <server/math/band/BandWrappers.h>
#include <Eigen/Dense>
#include <assert.h>
#include <server/common/logging.h>

namespace sail {
namespace BandedIrls {

struct Settings {
  int iterations = 30;
  double defaultDiagReg = 1.0e-15;
};

// This is a helper class to construct a least-squares problem
// where the left-hand-side of the resulting normal equations
// has a band structure.
//
// Such a banded problem appears when the maximum difference
// between indices in of unknowns appearing in the same squared
// cost to be minimized is limited to a small number, for instance
// if there is a squared term (1*x_{i} - 2*x_{i+1} + 1*x_{i+2})^2
// in the cost function that we are minimizing w.r.t. X = (x_1, ... , x_n),
// then we see that the index distance is at most 2 between any pair
// of unknowns in a term, so the banded matrix does not need to be
// wider than three. The a banded least-squares problem can also
// be seen as a composition of many small dense least-squares problems
// covering different short index ranges of the full vector to be optimized.
// This is reflected in the API, by the 'addNormalEquations' function that
// adds one such small subproblem.
//
// Note that we can solve for multiple right hand sides in one go.
class BandProblem {
public:
  int lhsDim() const {return _lhs.size();}
  int rhsDim() const {return _rhs.cols();}

  template <int rows, int leftCols, int rightCols>
  void addNormalEquations(
      int startAt,
      const Eigen::Matrix<double, rows, leftCols>& A,
      const Eigen::Matrix<double, rows, rightCols>& B) {
    for (int j = 0; j < leftCols; j++) {
      for (int k = 0; k <= j; k++) {
        double sum = 0.0;
        for (int l = 0; l < rows; l++) {
          sum += A(l, j)*A(l, k);
        }
        auto J = startAt + j;
        auto K = startAt + k;
        CHECK(J >= K);
        _lhs.add(J, K, sum);
      }
      for (int k = 0; k < rightCols; k++) {
        double sum = 0.0;
        for (int l = 0; l < rows; l++) {
          sum += A(l, j)*B(l, k);
        }
        _rhs(startAt + j, k) += sum;
      }
    }
  }

  BandProblem(int lhsDims,
      int rhsDims, int maxBlockSize,
      double initialDiagElement);

  MDArray2d solveAndInvalidate();
  bool empty() const {return _rhs.empty();}
private:
  SymmetricBandMatrixL<double> _lhs;
  MDArray<double, 2> _rhs;
};

// An array of Cost::Ptr together make up an optimization problem to
// be solved by the 'solve' function later in this file.
//
// In BandedIrlsUtils.hpp, there are implementations of this abstract class,
// of which StaticCost is the most basic one, representing a regular linear
// least-squares cost component.
class Cost {
public:
  typedef std::shared_ptr<Cost> Ptr;

  // For the first iteration, we may not have an initial solution.
  virtual void initialize(BandProblem* dst) = 0;

  virtual void constantApply(
      BandProblem* dst) const = 0;

  // Called for every iteration to build the new least-squares problem
  // to be solved.
  virtual void apply(
      int iteration,
      const MDArray2d& currentSolution,
      BandProblem* dst) {
    constantApply(dst);
  }

  // 1+(the highest index referred to in the optimized vector)
  virtual int minimumProblemDimension() const = 0;

  // If the index difference between any pair of coefficients
  // in a squared cost never exceeds n, then this function should
  // return 1+n.
  virtual int maximumDiagonalWidth() const = 0;

  // Usually 1, but in some problems, other values may be useful.
  // For instance, in some cases when fitting a 2d curve, the first
  // column can hold X coordinates and the second column can hold
  // Y coordinates.
  virtual int rightHandSideDimension() const {return 1;}

  virtual ~Cost() {}
};

struct Results {
  bool OK() const {return !X.empty();}
  MDArray2d X;
};

// This implements an optimization algorithm used to solve
// certain classes of nonlinear continuous optimization problems
// that internally can be solved as a series of sparse banded problems.
Results solve(
    const Settings& options,
    const Array<Cost::Ptr>& costs,
    const MDArray2d& Xinit = MDArray2d());

Results constantSolve(
    const Settings& settings,
    const Array<Cost::Ptr>& costs);

}
} /* namespace sail */

#endif /* SERVER_MATH_BANDEDIRLS_H_ */
