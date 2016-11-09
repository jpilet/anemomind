#include <device/anemobox/DispatcherFilter.h>

namespace sail {

Angle<double> DispatcherFilter::filterAngle(
    const DispatchAngleData *angles, Duration<> window) const {
  HorizontalMotion<double> accumulator = HorizontalMotion<double>::zero();
  double accumulatedWeight = 0;

  TimeStamp now = _dispatcher->currentTime();

  const TimedSampleCollection<Angle<double>>& angleValues =
    angles->dispatcher()->values();

  for (size_t i = 0; i < angleValues.size(); ++i) {
    Duration<> delta = now - angleValues.back(i).time;
    if (delta > window) {
      break;
    }
    double factor = 1 - delta.seconds() / window.seconds(); 
    accumulator = accumulator + HorizontalMotion<double>::polar(
        Velocity<double>::knots(factor), angleValues.back(i).value);
    accumulatedWeight += factor;
  }

  if (accumulatedWeight == 0) {
    // TODO: return something invalid.
    return Angle<>::degrees(0);
  }
  return accumulator.angle();
}

Velocity<double> DispatcherFilter::filterVelocity(
    DispatchVelocityData *velocities, Duration<> window) const {
  TimeStamp now = _dispatcher->currentTime();

  const TimedSampleCollection<Velocity<double>>& velocityValues =
    velocities->dispatcher()->values();
  Velocity<double> velocityAccumulator = Velocity<double>::knots(0);

  double velocityAccumulatedWeight = 0;
  for (size_t i = 0; i < velocityValues.size(); ++i) {
    const TimedValue<Velocity<double>>& timedValue = velocityValues.back(i);
    Duration<> delta = now - timedValue.time;
    if (delta > window) {
      break;
    }
    double factor = 1 - delta.seconds() / window.seconds(); 
    velocityAccumulator += timedValue.value.scaled(factor);
    velocityAccumulatedWeight += factor;
  }
  if (velocityAccumulatedWeight == 0) {
    // TODO: return something invalid.
    return Velocity<>::knots(0);
  }

  return velocityAccumulator.scaled(1.0 / velocityAccumulatedWeight);
}

}  // namespace sail
