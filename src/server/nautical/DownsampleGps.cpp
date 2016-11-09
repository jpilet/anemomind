

#include <server/nautical/DownsampleGps.h>

namespace sail {

namespace {

bool validPos(const GeographicPosition<double>& pos) {
  return fabs(pos.lat()) > Angle<double>::degrees(1e-5)
    && fabs(pos.lon()) > Angle<double>::degrees(1e-5);
}

TimedSampleCollection<GeographicPosition<double> >
  downSamplePosTo1Hz(const SampledSignal<GeographicPosition<double> >& pos) {
  TimedSampleCollection<GeographicPosition<double> > dst;

  Duration<> almostOneSec = Duration<>::seconds(.99);

  for (int i = 0; i < pos.size(); ++i) {
    if (validPos(pos[i].value)
        && (dst.size() == 0
            || ((pos[i].time - dst.lastTimeStamp()) >= almostOneSec))) {
      dst.append(pos[i]);
    }
  }
  return dst;
}

}  // namespace

NavDataset downSampleGpsTo1Hz(const NavDataset& navs) {
  return navs.replaceChannel<GeographicPosition<double> >(
      GPS_POS,
      navs.dispatcher()->get<GPS_POS>()->source() + " resampled to 1Hz",
      downSamplePosTo1Hz(navs.samples<GPS_POS>()).samples());
}

}  // namespace sail
