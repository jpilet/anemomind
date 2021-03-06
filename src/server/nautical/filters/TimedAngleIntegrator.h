/*
 * AngleIntegrator.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_TIMED_ANGLE_INTEGRATOR_H_
#define SERVER_NAUTICAL_FILTERS_TIMED_ANGLE_INTEGRATOR_H_

#include <server/nautical/filters/TimedValueIntegrator.h>
#include <Eigen/Dense>

namespace sail {

template <typename AngleIterator>
Array<TimedValue<Eigen::Vector2d> > anglesToUnitVectorArray(
    AngleIterator begin, AngleIterator end) {
  auto n = computeIteratorRangeSize(begin, end);
  auto unitVel = Velocity<double>::knots(1.0);
  ArrayBuilder<TimedValue<Eigen::Vector2d> > dst(n);
  for (auto i = begin; i != end; i++) {
    auto x = *i;
    auto vec = Eigen::Vector2d(cos(x.value), sin(x.value));
    dst.add(TimedValue<Eigen::Vector2d>(x.time, vec));
  }
  return dst.get();
}

// Angles are special because they are cyclic. We need to treat them
// differently. It tries to provide the same interface as the general
// TimedValueIntegrator.
template <>
class TimedValueIntegrator<Angle<double> > {
public:
  typedef ValueAndDurationToNearestSample<Angle<double> > Value;

  TimedValueIntegrator<Angle<double> >() {}

  static TimedValueIntegrator<Angle<double> > makeFromArray(
      const Array<TimedValue<Angle<double> > > &values);
  Optional<Value> computeAverage(TimeStamp from, TimeStamp to) const;
  Optional<Value> interpolate(TimeStamp t) const;

  template <typename AngleIterator>
  static TimedValueIntegrator<Angle<double> > make(AngleIterator begin, AngleIterator end) {
    return TimedValueIntegrator<Angle<double> >(
        TimedValueIntegrator<Eigen::Vector2d>::makeFromArray(
            anglesToUnitVectorArray(begin, end)));
  }

  bool empty() const {
    return _itg.empty();
  }
private:

  TimedValueIntegrator<Angle<double> >(const TimedValueIntegrator<Eigen::Vector2d> &itg) :
    _itg(itg) {}

  TimedValueIntegrator<Eigen::Vector2d> _itg;
};

}


#endif /* SERVER_NAUTICAL_FILTERS_TIMED_ANGLE_INTEGRATOR_H_ */
