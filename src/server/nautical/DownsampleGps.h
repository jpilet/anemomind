#ifndef NAUTICAL_DOWNSAMPLE_GPS
#define NAUTICAL_DOWNSAMPLE_GPS

#include <server/nautical/NavDataset.h>

namespace sail {

// Returns a new NavDataset with a merged and downsampled GPS_POS
// channel.
NavDataset downSampleGpsTo1Hz(const NavDataset& navs);

}  // namespace sail

#endif  // NAUTICAL_DOWNSAMPLE_GPS
