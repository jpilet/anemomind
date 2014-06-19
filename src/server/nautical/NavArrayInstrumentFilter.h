#ifndef NAUTICAL_NAV_ARRAY_INSTRUMENT_FILTER_H
#define NAUTICAL_NAV_ARRAY_INSTRUMENT_FILTER_H

#include <server/nautical/Nav.h>

class NavArrayInstrumentFilter {
 public:
  NavArrayInstrumentFilter(Array<Nav> navs, int pos,
                             Duration<> averageWindow);

  Angle<double> awa() const { return _measures.awa(); }
  Velocity<double> aws() const { return _measures.aws(); }
  Angle<double> magHdg() const { return _measures.magHdg(); }
  Velocity<double> watSpeed() const { return _measures.watSpeed(); }
  Velocity<double> gpsSpeed() const { return _measures.gpsSpeed(); }
  Angle<double> gpsBearing() const { return _measures.gpsBearing(); }

 private:
  Nav _measures;
};

#endif // NAUTICAL_NAV_ARRAY_INSTRUMENT_FILTER_H
