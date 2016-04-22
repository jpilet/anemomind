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

TimedAngleIntegrator TimedAngleIntegrator::makeFromArray(
    const Array<TimedValue<Angle<double> > > &values) {
  return TimedAngleIntegrator(
      TimedValueIntegrator<Eigen::Vector2d>::makeFromArray(
          anglesToUnitVectorArray(values.begin(), values.end())));
}

namespace {
  Optional<TimedAngleIntegrator::Value> vecToAngle(
      const TimedValueIntegrator<Eigen::Vector2d>::Value &v) {
    auto rads = atan2(double(v.value(1)), double(v.value(0)));
    if (std::isfinite(rads)) {
      return Optional<TimedAngleIntegrator::Value>(
        TimedAngleIntegrator::Value{
        v.maxDuration,
        Angle<double>::radians(rads)
      });
    }
    return Optional<TimedAngleIntegrator::Value>();
  }

  Optional<TimedAngleIntegrator::Value> optionalVecToAngle(
      const Optional<TimedValueIntegrator<Eigen::Vector2d>::Value > &v) {
    if (v.defined()) {
      return vecToAngle(v.get());
    }
    return Optional<TimedAngleIntegrator::Value>();
  }
}

Optional<TimedAngleIntegrator::Value> TimedAngleIntegrator::computeAverage(
    TimeStamp from, TimeStamp to) const {
  return optionalVecToAngle(_itg.computeAverage(from, to));
}

Optional<TimedAngleIntegrator::Value> TimedAngleIntegrator::interpolate(
    TimeStamp t) const {
  return optionalVecToAngle(_itg.interpolate(t));
}



}

