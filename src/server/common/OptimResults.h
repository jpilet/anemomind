/*
 *  Created on: 2014-03-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Class to hold the solution vector and objective function value, when
 *  solving an minimization problem.
 */

#ifndef OPTIMRESULTS_H_
#define OPTIMRESULTS_H_

namespace sail {

class MinimizationResults {
 public:
  MinimizationResults();
  MinimizationResults(double value_, Arrayd X_) : _objfValue(value_),
      _X(X_.dup()) {}
  bool operator<(const MinimizationResults &other) const {return _objfValue < other._objfValue;}
  Arrayd X() const {return _X;}
  Arrayd objfValue() {return _objfValue;}
 private:
  double _objfValue;
  Arrayd _X;
};

} /* namespace sail */

#endif /* OPTIMRESULTS_H_ */
