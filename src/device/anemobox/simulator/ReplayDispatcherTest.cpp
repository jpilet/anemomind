#include <device/anemobox/simulator/ReplayDispatcher.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(ReplayDispatcherTest, SmokeTest) {
  ReplayDispatcher replay;
  Nav nav;

  nav.setTime(TimeStamp::parse("12/27/15 13:17:28"));
  nav.setGeographicPosition(
      GeographicPosition<double>(
          Angle<>::degrees(6.1234), Angle<>::degrees(43.123)));
  nav.setAwa(Angle<>::degrees(32));
  nav.setAws(Velocity<>::knots(15));
  nav.setGpsBearing(Angle<>::degrees(140));
  nav.setGpsSpeed(Velocity<>::knots(6));
  nav.setMagHdg(Angle<>::degrees(140));
  nav.setWatSpeed(Velocity<>::knots(6));

  replay.advanceTo(nav);

  EXPECT_EQ(replay.get<GPS_POS>()->dispatcher()->lastTimeStamp().toSecondsSince1970(),
            nav.time().toSecondsSince1970());

  EXPECT_EQ(replay.val<GPS_POS>().lat().degrees(),
            nav.geographicPosition().lat().degrees());

  EXPECT_EQ(replay.val<GPS_POS>().lon().degrees(),
            nav.geographicPosition().lon().degrees());

  EXPECT_EQ(replay.val<GPS_BEARING>().degrees(),
            nav.gpsBearing().degrees());

  EXPECT_EQ(replay.val<GPS_SPEED>().knots(),
            nav.gpsSpeed().knots());

  EXPECT_EQ(replay.val<MAG_HEADING>().degrees(),
            nav.magHdg().degrees());

  EXPECT_EQ(replay.val<WAT_SPEED>().knots(),
            nav.watSpeed().knots());
}
