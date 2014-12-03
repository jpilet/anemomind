/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Flow.h"

namespace sail {

namespace {
  Velocity<double> eval(const Flow::VelocityFunction &fun,
      const Flow::ProjectedPosition &p, Duration<double> t) {
    if (fun) {
      return fun(p, t);
    }
    return Velocity<double>::knots(0.0);
  }

  Flow::VelocityFunction operator+ (const Flow::VelocityFunction &a, const Flow::VelocityFunction &b) {
    if (!bool(a)) {
      return b;
    } else if (!bool(b)) {
      return a;
    }

    return [=](const Flow::ProjectedPosition &p, Duration<double> t) {
      return a(p, t) + b(p, t);
    };
  }
}

Flow::Flow(VelocityFunction x, VelocityFunction y) {
  _funs[0] = x;
  _funs[1] = y;
}

Flow Flow::constant(const HorizontalMotion<double> &m) {
  return constant(m[0], m[1]);
}

Flow Flow::constant(Velocity<double> x, Velocity<double> y) {
  return Flow([=](const ProjectedPosition &p, Duration<double> t) {
                  return x;
              },[=](const ProjectedPosition &p, Duration<double> t) {
                  return y;
              });
}


HorizontalMotion<double> Flow::operator() (const ProjectedPosition &p, Duration<double> t) const {
  return HorizontalMotion<double>(eval(_funs[0], p, t), eval(_funs[1], p, t));
}

Flow Flow::operator+ (const Flow &other) const {
  return Flow(_funs[0] + other._funs[0], _funs[1] + other._funs[1]);
}

std::function<HorizontalMotion<double>(Flow::ProjectedPosition, Duration<double>)> Flow::asFunction() const {
  return [=](const ProjectedPosition &p, Duration<double> t) {
    return (*this)(p, t);
  };
}


} /* namespace mmm */
