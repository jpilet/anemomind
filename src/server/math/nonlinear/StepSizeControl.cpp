/*
 * StepSizeControl.cpp
 *
 *  Created on: 29 Dec 2016
 *      Author: jonas
 */

#include "StepSizeControl.h"

namespace sail {

StepSizeControl::StepSizeControl() : _initial(NAN) {}

StepSizeControl::StepSizeControl(double initialStepSize) :
    _initial(log(initialStepSize)) {
  _acc = QuadForm<2, 1>::fitLine(log(0.5*initialStepSize), -1.0)
      + QuadForm<2, 1>::fitLine(log(2.0*initialStepSize), 1.0);
}

StepSizeControl::StepSize StepSizeControl::proposeStepSize() const {
  double km[2] = {0, 0};
  if (_acc.minimize2x1(km)) {
    double k = km[0];
    double m = km[1];
    double result = -m/k;
    return StepSize(std::isfinite(result)? result : _initial);
  } else {
    return StepSize(_initial);
  }
}

void StepSizeControl::reportGoodStepSize(const StepSize &s) {
  reportStepSize(s, -1.0);
}

void StepSizeControl::reportBadStepSize(const StepSize &s) {
  reportStepSize(s, 1.0);
}

void StepSizeControl::reportStepSize(const StepSize &s, double h) {
  _acc += QuadForm<2, 1>::fitLine(s._logValue, h);
}



} /* namespace sail */
