/*
 * TimedObservation.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_TIMEDOBSERVATION_H_
#define SERVER_MATH_NONLINEAR_TIMEDOBSERVATION_H_

#include <Eigen/Dense>

namespace sail {

// Used to represent observations in some optimization context
template <int N, typename T = double>
struct TimedObservation {
  typedef Eigen::Matrix<T, N, 1> Vec;

  TimedObservation() : order(-1), time(NAN) {}
  TimedObservation(int o, T t, const Vec &v) : order(o), time(t), value(v) {}

  int order; // For instance 0 can mean position and 1 can mean velocity
  T time;
  Vec value;

  bool operator<(const TimedObservation<N, T> &other) const {
    return time < other.time;
  }
};



}

#endif /* SERVER_MATH_NONLINEAR_TIMEDOBSERVATION_H_ */
