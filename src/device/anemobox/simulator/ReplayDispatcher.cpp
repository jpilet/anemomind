#include <device/anemobox/simulator/ReplayDispatcher.h>

namespace sail {

void ReplayDispatcher::advanceTo(const Nav& nav) {
  const std::string source(sourceName());

  // Replayed data should not override computed true wind.
  setSourcePriority(source, -2);

  _replayTime = nav.time();

  if (nav.hasApparentWind()) {
    publishValue(AWA, source, nav.awa());
    publishValue(AWS, source, nav.aws());
  }

  if (nav.hasMagHdg()) {
    publishValue(MAG_HEADING, source, nav.magHdg());
  }

  publishValue(GPS_POS, source, nav.geographicPosition());
  publishValue(GPS_BEARING, source, nav.gpsBearing());
  publishValue(GPS_SPEED, source, nav.gpsSpeed());

  if (nav.hasWatSpeed()) {
    publishValue(WAT_SPEED, source, nav.watSpeed());
  }

  if (nav.hasExternalTrueWind()) {
    publishValue(TWA, source, nav.externalTwa());
    publishValue(TWS, source, nav.externalTws());
  }
}

}  // namespace sail
