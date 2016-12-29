/*
 * StepSizeControl.h
 *
 *  Created on: 29 Dec 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_STEPSIZECONTROL_H_
#define SERVER_MATH_NONLINEAR_STEPSIZECONTROL_H_

#include <server/math/QuadForm.h>

namespace sail {

class StepSizeControl {
public:
  class StepSize {
  public:
    StepSize() : _logValue(NAN) {}
    double value() const {return exp(_logValue);}
  private:
    StepSize(double logv) : _logValue(logv) {}
    double _logValue;
  };

  StepSizeControl();
  StepSizeControl(double initialStepSize);
  StepSize proposeStepSize() const;
  void reportGoodStepSize(const StepSize &s);
  void reportBadStepSize(const StepSize &s);
private:
  QuadForm<2, 1, double> _acc;
  double _initial;
  void reportStepSize(const StepSize &s, double h);
};

} /* namespace sail */

#endif /* SERVER_MATH_NONLINEAR_STEPSIZECONTROL_H_ */
