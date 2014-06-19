#include <server/nautical/NavArrayInstrumentFilter.h>

namespace {

Nav averageNavs(Array<Nav> past, Duration<> duration) {
  CHECK_LT(0, past.size());

  HorizontalMotion<double> boatMotion(
      Velocity<double>::metersPerSecond(0),
      Velocity<double>::metersPerSecond(0));

  HorizontalMotion<double> apparentWind(
      Velocity<double>::metersPerSecond(0),
      Velocity<double>::metersPerSecond(0));

  HorizontalMotion<double> waterMotion(boatMotion);

  int n = 0;
  for (int i = past.size() - 1; i >= 0; --i) {
    const Nav& entry = past[i];
    if ((past.last().time() - entry.time()) < duration) {
      boatMotion = boatMotion + HorizontalMotion<double>::polar(
          entry.gpsSpeed(), entry.gpsBearing());

      apparentWind = apparentWind + HorizontalMotion<double>::polar(
          entry.aws(), entry.awa());

      waterMotion = waterMotion +  HorizontalMotion<double>::polar(
          entry.watSpeed(), entry.magHdg());
      ++n;
    }
  }

  boatMotion = boatMotion.scaled(1.0 / n);
  apparentWind = apparentWind.scaled(1.0 / n);
  waterMotion = waterMotion.scaled(1.0 / n);

  Nav result;
  result.setAwa(apparentWind.angle());
  result.setAws(apparentWind.norm());
  result.setGpsSpeed(boatMotion.norm());
  result.setGpsBearing(boatMotion.angle());
  result.setMagHdg(waterMotion.angle());
  result.setWatSpeed(waterMotion.norm());
  return result;
}

NavArrayInstrumentFilter::NavArrayInstrumentFilter(
    Array<Nav> navs, int pos, Duration<> averageWindow) {
  _measures = averageNavs(navs.slice(0, pos), averageWindow);
}

}  // namespace
