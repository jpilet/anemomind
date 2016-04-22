/*
 * AngleIntegrator.h
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_TIMEDANGLEINTEGRATOR_H_
#define SERVER_NAUTICAL_FILTERS_TIMEDANGLEINTEGRATOR_H_

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
class TimedAngleIntegrator {
public:
  typedef TimedValueIntegrator<Angle<double> >::Value Value;

  TimedAngleIntegrator() {}

  static TimedAngleIntegrator makeFromArray(
      const Array<TimedValue<Angle<double> > > &values);
  Optional<Value> computeAverage(TimeStamp from, TimeStamp to) const;
  Optional<Value> interpolate(TimeStamp t) const;

  template <typename AngleIterator>
  static TimedAngleIntegrator make(AngleIterator begin, AngleIterator end) {
    return TimedAngleIntegrator(
        TimedValueIntegrator<Eigen::Vector2d>::makeFromArray(
            anglesToUnitVectorArray(begin, end)));
  }

  bool empty() const {
    return _itg.empty();
  }
private:

  TimedAngleIntegrator(const TimedValueIntegrator<Eigen::Vector2d> &itg) :
    _itg(itg) {}

  TimedValueIntegrator<Eigen::Vector2d> _itg;
};

}


#endif /* SERVER_NAUTICAL_FILTERS_TIMEDANGLEINTEGRATOR_H_ */
