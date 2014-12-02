/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FLOW_H_
#define FLOW_H_

#include <server/nautical/GeographicReference.h>

namespace sail {

/*
 * A class that can be used to build
 * vectorfields easily to represent wind and
 * current in space and time. It replaces
 * the less general class FlowField
 * that is removed in branch jo-testcase.
 */
class Flow {
 public:
  typedef GeographicReference::ProjectedPosition ProjectedPosition;
  typedef std::function<Velocity<double>(ProjectedPosition, Duration<double>)> VelFun;

  Flow();
  Flow(VelFun x, VelFun y);
  static Flow constant(const HorizontalMotion<double> &m) const;

  HorizontalMotion<double> operator() (const ProjectedPosition &p, Duration<double> t) const;
  Flow operator+ (const Flow &other) const;

  VelFun make() const;
 private:
  VelFun _funs[2];
};

}

#endif /* FLOW_H_ */
