#ifndef NAUTICAL_FILTER_GPS_SEGMENTS
#define NAUTICAL_FILTER_GPS_SEGMENTS

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/nautical/ValidPeriods.h>
#include <server/nautical/NavDataset.h>

namespace sail {

struct GpsLogicParams {
  Duration<double> maxLinkTime = Duration<double>::seconds(6);
  Length<> maxLinkDist = Length<>::meters(100);
  Velocity<> maxLinkSpeed = Velocity<>::knots(80);
  bool logRejectedPoints = false;
};

// Returns cleaned GPS tracks
NavDataset findGpsSegments(const NavDataset& in,
                           const GpsLogicParams& params,
                           StatusTimedVector *segments
                           );

}  // namespace sail

#endif  // NAUTICAL_FILTER_GPS_SEGMENTS
