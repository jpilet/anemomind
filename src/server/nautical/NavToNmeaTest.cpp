
#include <server/nautical/NavToNmea.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <gtest/gtest.h>

#include <iostream>

using namespace sail;


TEST(NavToNmeaTest, SimpleRMC) {
  Nav nav;

  // $GPRMC,220516,A,5133.82,N,00042.24,W,73.8,231.8,130614,004.2,W*XX

  nav.setTime(TimeStamp::UTC(2015, 6, 13, 22, 5, 16));
  nav.setGeographicPosition(
      GeographicPosition<double>(Angle<>::degrees(-(0 + 42.24 / 60)),
                                 Angle<>::degrees(51 + 33.82 / 60)));
  nav.setGpsSpeed(Velocity<>::knots(73.8));
  nav.setGpsBearing(Angle<>::degrees(231.8));

  std::string sentence = nmeaRmc(nav);

  NmeaParser parser;

  for (char c: sentence) {
    parser.processByte(c);
  }

  EXPECT_EQ(1, parser.numSentences());
  EXPECT_EQ(nav.time(), parser.timestamp());
  EXPECT_EQ(nav.geographicPosition().lon().degrees(), parser.pos().lon.toDouble());
  EXPECT_EQ(nav.geographicPosition().lat().degrees(), parser.pos().lat.toDouble());
  EXPECT_NEAR(nav.gpsBearing().degrees(), parser.gpsBearing().degrees(), 1);
  EXPECT_NEAR(nav.gpsSpeed().knots(), parser.gpsSpeed().knots(), 1);
}
