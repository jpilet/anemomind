/*
 * LsqNashSolver.h
 *
 *  Created on: 6 Jan 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_LSQNASHSOLVER_H_
#define SERVER_MATH_NONLINEAR_LSQNASHSOLVER_H_

#include <Eigen/Dense>
#include <ceres/jet.h>

namespace sail {
namespace LsqNashSolver {

class SubFunction {
public:
  virtual void eval(
      const double *X, double *dst) const = 0;
  virtual void eval(const double *X,
      double *dst, int jacobianRow,
      std::vector<Eigen::Triplet<double>> *dst) = 0;
  virtual ~SubFunction() {}
};

template <typename F, int ... InputDim>
class ADSubFunction {
  ADSubFunction(Array<Spani> ... inputSpans) :
    _inputSpans({inputSpans...}) {}
public:
private:
  Array<Spani> _inputSpans;
};

}
}

#endif /* SERVER_MATH_NONLINEAR_LSQNASHSOLVER_H_ */
