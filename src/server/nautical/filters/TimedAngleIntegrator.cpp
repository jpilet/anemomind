/*
 * AngleIntegrator.cpp
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#include <server/nautical/filters/TimedAngleIntegrator.h>
#include <iostream>
#include <server/common/ArrayIO.h>

namespace sail {

TimedValueIntegrator<Angle<double> > TimedValueIntegrator<Angle<double> >::makeFromArray(
    const Array<TimedValue<Angle<double> > > &values) {
  return TimedValueIntegrator<Angle<double> >(
      TimedValueIntegrator<Eigen::Vector2d>::makeFromArray(
          anglesToUnitVectorArray(values.begin(), values.end())));
}

namespace {
  Optional<TimedValueIntegrator<Angle<double> >::Value> vecToAngle(
      const TimedValueIntegrator<Eigen::Vector2d>::Value &v) {
    auto rads = atan2(double(v.value(1)), double(v.value(0)));
    if (std::isfinite(rads)) {
      return Optional<TimedValueIntegrator<Angle<double> >::Value>(
        TimedValueIntegrator<Angle<double> >::Value{
        v.duration,
        Angle<double>::radians(rads)
      });
    }
    return Optional<TimedValueIntegrator<Angle<double> >::Value>();
  }

  Optional<TimedValueIntegrator<Angle<double> >::Value> optionalVecToAngle(
      const Optional<TimedValueIntegrator<Eigen::Vector2d>::Value > &v) {
    if (v.defined()) {
      return vecToAngle(v.get());
    }
    return Optional<TimedValueIntegrator<Angle<double> >::Value>();
  }
}

Optional<TimedValueIntegrator<Angle<double> >::Value> TimedValueIntegrator<Angle<double> >::computeAverage(
    TimeStamp from, TimeStamp to) const {
  return optionalVecToAngle(_itg.computeAverage(from, to));
}

Optional<TimedValueIntegrator<Angle<double> >::Value> TimedValueIntegrator<Angle<double> >::interpolate(
    TimeStamp t) const {
  return optionalVecToAngle(_itg.interpolate(t));
}



}

