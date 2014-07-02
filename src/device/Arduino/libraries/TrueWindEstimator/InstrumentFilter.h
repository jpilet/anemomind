#ifndef DEVICE_INSTRUMENT_FILTER_H
#define DEVICE_INSTRUMENT_FILTER_H

#include "../PhysicalQuantity/PhysicalQuantity.h"

#ifdef ON_SERVER
#include <server/nautical/Nav.h>
#endif

namespace sail {

// This class is in charge of applying a temporal filter on measured data.
// It also acts as a buffer between either a nmeaparser or a Array<Nav>.
template <class T>
class InstrumentFilter {
 public:
   typedef T type;

  Angle<T> awa() const { return _awa; }
  Velocity<T> aws() const { return _aws; }
  Angle<T> magHdg() const { return _magHdg; }
  Velocity<T> watSpeed() const { return _watSpeed; }
  Velocity<T> gpsSpeed() const { return _gpsSpeed; }
  Angle<T> gpsBearing() const { return _gpsBearing; }

  void setAwa(Angle<T> awa_) {_awa = awa_;}
  void setAws(Velocity<T> aws_) {_aws = aws_;}
  void setMagHdg(Angle<T> magHdg_) {_magHdg = magHdg_;}
  void setWatSpeed(Velocity<T> watSpeed_) {_watSpeed = watSpeed_;}
  void setGpsSpeed(Velocity<T> gpsSpeed_) {_gpsSpeed = gpsSpeed_;}
  void setGpsBearing(Angle<T> gpsBearing_) {_gpsBearing = gpsBearing_;}

 private:
  Angle<T> _awa;
  Velocity<T> _aws;
  Angle<T> _magHdg;
  Velocity<T> _watSpeed;
  Velocity<T> _gpsSpeed;
  Angle<T> _gpsBearing;
};

#ifdef ON_SERVER
namespace {
InstrumentFilter<double> makeFilter(const Array<Nav>& navs) {
  InstrumentFilter<double> filter;
  for (const Nav& nav : navs) {
    filter.setAwa(nav.awa());
    filter.setAws(nav.aws());
    filter.setMagHdg(nav.magHdg());
    filter.setWatSpeed(nav.watSpeed());
    filter.setGpsBearing(nav.gpsBearing());
    filter.setGpsSpeed(nav.gpsSpeed());
  }
  return filter;
}

}  // namespace
#endif

}  // namespace sail

#endif // DEVICE_INSTRUMENT_FILTER_H

