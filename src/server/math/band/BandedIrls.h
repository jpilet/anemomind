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

class Cost {
public:
  typedef std::shared_ptr<Cost> Ptr;

  virtual void initialize(BandProblem* dst) = 0;

  virtual void apply(
      int iteration,
      const MDArray2d& currentSolution,
      BandProblem* dst) = 0;

  virtual int minimumProblemDimension() const = 0;

  virtual int maximumDiagonalWidth() const = 0;

  virtual int rightHandSideDimension() const {return 1;}

  virtual ~Cost() {}
};

struct Results {

  bool OK() const {return !X.empty();}

  MDArray2d X;
};

Results solve(
    const Settings& options,
    const Array<Cost::Ptr>& costs,
    const MDArray2d& Xinit = MDArray2d());

}
} /* namespace sail */

#endif /* SERVER_MATH_BANDEDIRLS_H_ */
