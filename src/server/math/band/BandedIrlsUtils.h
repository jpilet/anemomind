/*
 * BandedIrlsUtils.h
 *
 *  Created on: 30 Mar 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_BAND_BANDEDIRLSUTILS_H_
#define SERVER_MATH_BAND_BANDEDIRLSUTILS_H_

#include <server/math/band/BandedIrls.h>
#include <server/math/OutlierRejector.h>
#include <server/common/LineKM.h>

namespace sail {
namespace BandedIrls {

class ExponentialWeighting {
public:
  ExponentialWeighting(int iterations, double firstWeight, double lastWeight) :
    _weighting(0, iterations-1, log(firstWeight), log(lastWeight)) {}
  double evaluate(int iteration) const {
    return exp(_weighting(iteration));
  }
private:
  LineKM _weighting;
};

template <int rows, int cols, int rhs=1>
class StaticCost : public Cost {
public:
  StaticCost(int at,
      const Eigen::Matrix<double, rows, cols>& A,
      const Eigen::Matrix<double, rows, rhs>& B) : _at(at), _A(A), _B(B) {}

  void initialize(BandProblem* dst) override {
    dst->addNormalEquations(_at, _A, _B);
  }

  virtual void apply(
      int iteration,
      const MDArray2d& currentSolution,
      BandProblem* dst) override {
    dst->addNormalEquations(_at, _A, _B);
  }

  int minimumProblemDimension() const override {
    return _at + cols;
  }

  int maximumDiagonalWidth() const override {
    return cols;
  }

  int rightHandSideDimension() const override {
    return rhs;
  }
private:
  int _at;
  Eigen::Matrix<double, rows, cols> _A;
  Eigen::Matrix<double, rows, rhs> _B;
};


template <int rows, int cols, int rhs=1, typename SourceData=void>
class RobustCost : public Cost {
public:
  RobustCost(int at,
      const Eigen::Matrix<double, rows, cols>& A,
      const Eigen::Matrix<double, rows, rhs>& B,
      const ExponentialWeighting& weighting,
      double inlierThreshold) : _at(at), _A(A), _B(B),
      _weighting(weighting) {
    OutlierRejector::Settings s;
    s.initialAlpha = weighting.evaluate(0);
    s.initialBeta = s.initialAlpha;
    s.sigma = inlierThreshold;
    _rejector = OutlierRejector(s);
  }

  void initialize(BandProblem* dst) override {
    auto w = _rejector.computeWeight();
    addWithWeight(w, dst);
  }

  bool isInlier(const MDArray2d& currentSolution) const {
    return computeError(currentSolution) < _rejector.sigma();
  }

  void apply(
      int iteration,
      const MDArray2d& currentSolution,
      BandProblem* dst) override {
    _rejector.update(
        _weighting.evaluate(iteration),
        computeError(currentSolution));
    addWithWeight(_rejector.computeWeight(), dst);
  }

  int minimumProblemDimension() const override {
    return _at + cols;
  }

  int maximumDiagonalWidth() const override {
    return cols;
  }

  int rightHandSideDimension() const override {return rhs;}
private:
  void addWithWeight(double w, BandProblem *dst) const {
    dst->addNormalEquations<rows, cols, rhs>(_at, w*_A, w*_B);
  }

  double computeError(const MDArray2d& X) const {
    double sum = 0.0;
    for (int i = 0; i < rhs; i++) {
      sum += (_A*Eigen::Map<Eigen::Matrix<
          double, cols, 1>>(X.getPtrAt(_at, i))
          - _B.template block<rows, 1>(0, i)).squaredNorm();
    }
    return sqrt(sum);
  }

  int _at;
  Eigen::Matrix<double, rows, cols> _A;
  Eigen::Matrix<double, rows, rhs> _B;
  ExponentialWeighting _weighting;
  OutlierRejector _rejector;
};

}
} /* namespace sail */

#endif /* SERVER_MATH_BAND_BANDEDIRLSUTILS_H_ */
