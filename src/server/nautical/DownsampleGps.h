#ifndef NAUTICAL_DOWNSAMPLE_GPS
#define NAUTICAL_DOWNSAMPLE_GPS

#include <server/nautical/NavDataset.h>

namespace sail {

NavDataset removeStrangeGpsPositions(const NavDataset& ds);

// Returns a new NavDataset with a merged and downsampled GPS_POS
// channel.
NavDataset downSampleGpsTo1Hz(const NavDataset& navs);

TimedSampleCollection<GeographicPosition<double>>::TimedVector
  getCleanGpsPositions(const NavDataset &ds, const std::string &src);

}  // namespace sail

#endif  // NAUTICAL_DOWNSAMPLE_GPS
