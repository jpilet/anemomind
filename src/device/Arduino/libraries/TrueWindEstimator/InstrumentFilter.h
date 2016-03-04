#ifndef DEVICE_INSTRUMENT_FILTER_H
#define DEVICE_INSTRUMENT_FILTER_H

#include "../PhysicalQuantity/PhysicalQuantity.h"

#ifdef ON_SERVER
#include <server/nautical/NavCompatibility.h>
#include <server/common/TimeStamp.h>
#endif

namespace sail {

// This class is in charge of applying a temporal filter on measured data.
// It also acts as a buffer between either a nmeaparser or a NavDataset
template <class T, class TimeStamp, class Duration>
class InstrumentFilter {
 public:
   typedef T type;

  Angle<T> awa() const { return _appWind._motion.angle(); }
  Velocity<T> aws() const { return _appWind._motion.norm(); }
  Angle<T> magHdg() const { return _magWater._motion.angle(); }
  Velocity<T> watSpeed() const { return _magWater._motion.norm(); }
  Velocity<T> gpsSpeed() const { return _gps._motion.norm(); }
  Angle<T> gpsBearing() const { return _gps._motion.angle(); }

  HorizontalMotion<T> gpsMotion() const { return _gps._motion; }

  void setAw(Angle<T> awa, Velocity<T> aws, TimeStamp ts) {
    update(ts, awa, aws, &_appWind);
  }

  void setMagHdgWatSpeed(Angle<T> magHdg, Velocity<T> watSpeed, TimeStamp ts) {
    update(ts, magHdg, watSpeed, &_magWater);
  }

  void setGps(Angle<T> gpsBearing, Velocity<T> gpsSpeed, TimeStamp ts) {
    update(ts, gpsBearing, gpsSpeed, &_gps);
  }

  TimeStamp oldestUpdate() const {
    return min(_appWind._time, min(_magWater._time, _gps._time));
  }

 private:
  struct TimedMotion {
    TimeStamp _time;
    HorizontalMotion<T> _motion;

    TimedMotion() : _motion(HorizontalMotion<T>::zero()) { }
  };
  static void update(TimeStamp time,
              Angle<T> angle,
              Velocity<T> speed,
              TimedMotion *dest);

  TimedMotion _appWind;
  TimedMotion _magWater;
  TimedMotion _gps;
};

template <class T, class TimeStamp, class Duration>
void InstrumentFilter<T, TimeStamp, Duration>::update(
    TimeStamp time, Angle<T> angle, Velocity<T> speed, TimedMotion *dest) {
  HorizontalMotion<T> motion = HorizontalMotion<T>::polar(speed, angle);

#ifdef ON_SERVER
  if (isNaN(angle) || isNaN(speed)) {
    return;
  }
#endif

  Duration delta;
  if (dest->_time.undefined() ||
      (delta = fabs(time - dest->_time)) > Duration::seconds(20)) {
    // reset
    dest->_motion = motion;
  } else {
    // The higher this constant, the more filtering will be applied.
    // This value has been manually tuned. TODO: compute it automatically.
    //
    // The following line seems to cause a bug. TODO: activate it again
    // when a proper testing framework can check its validity.
    // T factor = pow(T(.6), T(min(1.0, delta.seconds())));
    T factor(.7);
    dest->_motion = dest->_motion.scaled(factor) + motion.scaled(T(1) - factor);
  }
  dest->_time = time;
}


#ifdef ON_SERVER
namespace {

typedef InstrumentFilter<double, sail::TimeStamp, Duration<double> > ServerFilter;

ServerFilter makeFilter(
    const NavDataset& navs) {
  ServerFilter filter;

  for (const Nav& nav : NavCompat::Range(navs)) {
    filter.setAw(nav.awa(), nav.aws(), nav.time());
    filter.setMagHdgWatSpeed(nav.magHdg(), nav.watSpeed(), nav.time());
    filter.setGps(nav.gpsBearing(), nav.gpsSpeed(), nav.time());
  }
  return filter;
}

}  // namespace
#endif

}  // namespace sail

#endif // DEVICE_INSTRUMENT_FILTER_H

