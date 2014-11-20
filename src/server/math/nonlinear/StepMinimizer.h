/*
 * StepMinimizer.h
 *
 *  Created on: 21 janv. 2014
 *      Author: jonas
 *
 *
 * Minimizes a function
 * 	f: R -> R
 * by trying discrete steps and gradually reducing the step size.
 */

#ifndef STEPMINIMIZER_H_
#define STEPMINIMIZER_H_

#include <functional>

namespace sail {

class StepMinimizerState {
 public:
  StepMinimizerState();
  StepMinimizerState(double x, double step, double value) : _x(x), _step(step), _value(value) {}

  double getX() const {
    return _x;
  }
  double getStep() const {
    return _step;
  }
  double getValue() const {
    return _value;
  }

  void reduceStep() {
    _step *= 0.5;
  }
  void increaseStep() {
    _step *= 2.0;
  }

  double getLeft() const {
    return _x - _step;
  }
  double getRight() const {
    return _x + _step;
  }

  StepMinimizerState make(double x, double value) const {
    return StepMinimizerState(x, _step, value);
  }

  StepMinimizerState reevaluate(std::function<double(double)> fun) {
    return StepMinimizerState(_x, _step, fun(_x));
  }
 private:
  double _x, _step, _value;
};

class StepMinimizer {
 public:
  StepMinimizer();
  StepMinimizer(int maxIter);
  StepMinimizer(double lm, double rm, int maxiter) :
    _leftLimit(lm), _rightLimit(rm), _maxIter(maxiter) {}


  // Run the minimizer until there is a reduction in the function value.
  StepMinimizerState takeStep(StepMinimizerState state, std::function<double(double)> fun);

  // Minimize the function.
  StepMinimizerState minimize(StepMinimizerState state, std::function<double(double)> fun);

  // The acceptor function lets us incorporate additional criteria in order for a solution to be accepted.
  void setAcceptor(std::function<bool(double, double)> acceptor);
  double recommendedInitialStep() const {return _rightLimit - _leftLimit;}
  int maxiter() const {return _maxIter;}
 private:
  double _leftLimit, _rightLimit;
  int _maxIter;

  std::function<bool(double, double)> _acceptor;
};





}

#endif /* STEPMINIMIZER_H_ */
