#include <gtest/gtest.h>
#include <server/nautical/filters/gpsLogic.h>

using namespace sail;

namespace {

TimeStamp testTime(int seconds) {
  static TimeStamp base = TimeStamp::now();
  return base + Duration<>::seconds(seconds);
}

TimedValue<GeographicPosition<double>> pos(int t, double lat, double lon) {
  return TimedValue<GeographicPosition<double>>(
      testTime(t),
      GeographicPosition<double>(Angle<>::degrees(lat), Angle<>::degrees(lon)));
}

}  // namespace

TEST(GpsLogic, findGpsSegments) {
  TimedSampleCollection<GeographicPosition<double> >::TimedVector points{
    pos(0, 49.01, -6.02), // outlier
    pos(1, 49.01, -3.02), // outlier
    pos(2, 29.00101, 6.000209), // ok
    pos(3, 29.00102, 6.000208),
    pos(4, 29.00103, 6.000207),
    pos(5, 29.00104, 6.000206),
    pos(6, 29.00105, 6.000205)
  };

  NavDataset ds = NavDataset().addChannel<GeographicPosition<double>>(
      GPS_POS, "gps", points);

  GpsLogicParams params;
  StatusTimedVector segments;
  NavDataset filtered = findGpsSegments(ds, params, &segments);

  EXPECT_EQ(2, segments.size());
  EXPECT_EQ(StatusChange::toValid, segments[0].value);
  EXPECT_EQ(testTime(2), segments[0].time);
  EXPECT_EQ(StatusChange::toInvalid, segments[1].value);
  EXPECT_EQ(testTime(6), segments[1].time);
}

