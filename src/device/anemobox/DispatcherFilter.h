#ifndef DEVICE_ANEMOBOX_FILTER_H
#define DEVICE_ANEMOBOX_FILTER_H

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/ValueDispatcher.h>
#include <server/common/TimeStamp.h>

namespace sail {

struct DispatcherFilterParams {
  Duration<> apparentWindWindow;
  Duration<> waterMotionWindow;
  Duration<> gpsMotionWindow;

  DispatcherFilterParams()
    : apparentWindWindow(Duration<>::seconds(15)),
    waterMotionWindow(Duration<>::seconds(10)),
    gpsMotionWindow(Duration<>::seconds(3)) { }
};

// This class is in charge of applying a temporal filter on measured data.
// It adapts the Dispatcher to functions such as computeTrueWind.
class DispatcherFilter {
 public:
   typedef double type;

  // The dispatcher is expected to remain valid during
  // the lifetime of the DispatcherFilter.
  DispatcherFilter(Dispatcher* dispatcher,
                   DispatcherFilterParams params) : _dispatcher(dispatcher) { }

  Angle<> awa() const {
    return filterAngle(_dispatcher->get<AWA>(), _params.apparentWindWindow);
  }

  Velocity<> aws() const {
    return filterVelocity(_dispatcher->get<AWS>(), _params.apparentWindWindow);
  }

  Angle<> magHdg() const {
    return filterAngle(_dispatcher->get<MAG_HEADING>(), _params.waterMotionWindow);
  }

  Velocity<> watSpeed() const {
    return filterVelocity(_dispatcher->get<WAT_SPEED>(), _params.waterMotionWindow);
  }

  Velocity<> gpsSpeed() const {
    return filterVelocity(_dispatcher->get<GPS_SPEED>(), _params.gpsMotionWindow);
  }

  Angle<> gpsBearing() const {
    return filterAngle(_dispatcher->get<GPS_BEARING>(), _params.gpsMotionWindow);
  }

  HorizontalMotion<double> gpsMotion() const {
    return HorizontalMotion<double>::polar(
        gpsSpeed(), gpsBearing());
  }

 private:

  Angle<double> filterAngle(const DispatchAngleData *angles,
                            Duration<> window) const;
  Velocity<double> filterVelocity(DispatchVelocityData *velocities,
                                  Duration<> window) const;

  Dispatcher* _dispatcher;
  DispatcherFilterParams _params;
};

}  // namespace sail

#endif // DEVICE_ANEMOBOX_FILTER_H

