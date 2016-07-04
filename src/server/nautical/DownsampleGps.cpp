

#include <server/nautical/DownsampleGps.h>

namespace sail {

namespace {

TimedSampleCollection<GeographicPosition<double> >
  downSamplePosTo1Hz(const SampledSignal<GeographicPosition<double> >& pos) {
  TimedSampleCollection<GeographicPosition<double> > dst;

  for (int i = 0; i < pos.size(); ++i) {
    if (i == 0
        || ((pos[i].time - dst.lastTimeStamp()) >= Duration<>::seconds(.99))) {
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
