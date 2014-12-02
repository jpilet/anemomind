/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Flow.h"

namespace sail {

namespace {
  Velocity<double> eval(const Flow::VelFun &fun,
      const Flow::ProjectedPosition &p, Duration<double> t) {
    if (fun) {
      return fun(p, t);
    }
    return 0.0;
  }

  Flow::VelFun operator+ (const Flow::VelFun &a, const Flow::VelFun &b) {
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

Flow::Flow(VelFun x, VelFun y) {
  _funs[0] = x;
  _funs[1] = y;
}

Flow Flow::constant(const HorizontalMotion<double> &m) const {
  return Flow([=](const ProjectedPosition &p, Duration<double> t) {
                  return m[0];
              },[=](const ProjectedPosition &p, Duration<double> t) {
                  return m[1];
              });
}


HorizontalMotion<double> Flow::operator() (const ProjectedPosition &p, Duration<double> t) const {
  return HorizontalMotion<double>(eval(_funs[0], p, t), eval(_funs[1], p, t));
}

Flow Flow::operator+ (const Flow &other) const {
  return Flow(_funs[0] + other._funs[0], _funs[1] + other._funs[1]);
}

Flow::VelFun Flow::make() const {
  return [=](const ProjectedPosition &p, Duration<double> t) {
    return (*this)(p, t);
  };
}


} /* namespace mmm */
