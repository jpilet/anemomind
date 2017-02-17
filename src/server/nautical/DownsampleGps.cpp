

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

TimedSampleCollection<GeographicPosition<double>>::TimedVector
  getCleanGpsPositions(
    const NavDataset &ds, const std::string &src) {
  auto data = toTypedDispatchData<GPS_POS>(
      ds.dispatcher()->dispatchDataForSource(GPS_POS, src).get());
  TimedSampleCollection<GeographicPosition<double>>::TimedVector dst;
  if (!data) {
    LOG(ERROR) << "Conversion to GPS pos failed";
    return dst;
  }
  for (auto x: data->dispatcher()->values()) {
    if (validPos(x.value)) {
      dst.push_back(x);
    }
  }
  return dst;
}

}  // namespace

NavDataset removeStrangeGpsPositions(const NavDataset &ds) {
  auto sources = ds.sourcesForChannel(GPS_POS);
  NavDataset result = ds.stripChannel(GPS_POS);
  for (auto src: sources) {
    auto cleanData = getCleanGpsPositions(ds, src);
    result = result.addChannel<GeographicPosition<double>>(
        GPS_POS, src, cleanData);
  }
  return result;
}

NavDataset downSampleGpsTo1Hz(const NavDataset& navs) {
  return navs.replaceChannel<GeographicPosition<double> >(
      GPS_POS,
      navs.dispatcher()->get<GPS_POS>()->source() + " resampled to 1Hz",
      downSamplePosTo1Hz(navs.samples<GPS_POS>()).samples());
}

}  // namespace sail
