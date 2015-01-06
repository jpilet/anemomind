/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FLOW_H_
#define FLOW_H_

#include <server/nautical/GeographicReference.h>
#include <functional>

namespace sail {

class GnuplotExtra;

/*
 * A class that can be used to build
 * vectorfields easily to represent wind and
 * current in space and time.
 */
class Flow {
 public:
  typedef GeographicReference::ProjectedPosition ProjectedPosition;
  typedef std::function<Velocity<double>(ProjectedPosition, Duration<double>)> VelocityFunction;

  static VelocityFunction spatiallyChangingVelocity(
      Velocity<double> amplitude,
      Angle<double> angle, Length<double> period, Angle<double> phase);

  Flow() {}
  Flow(VelocityFunction x, VelocityFunction y);
  static Flow constant(const HorizontalMotion<double> &m);
  static Flow constant(Velocity<double> x, Velocity<double> y);

  HorizontalMotion<double> operator() (const ProjectedPosition &p, Duration<double> t) const;
  Flow operator+ (const Flow &other) const;

  std::function<HorizontalMotion<double>(ProjectedPosition, Duration<double>)> asFunction() const;

  void plot1d(int dim, ProjectedPosition fromPos, Duration<double> fromTime,
      ProjectedPosition   toPos, Duration<double>   toTime,
      GnuplotExtra *dst);
 private:
  VelocityFunction _funs[2];
};

}

#endif /* FLOW_H_ */
