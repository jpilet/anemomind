#include "../NmeaParser.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <gtest/gtest.h>

using namespace sail;

double fabs(double a) {
  return (a< 0 ? -a : a);
}

NmeaParser::NmeaSentence sendSentence(const char *sentence, NmeaParser *parser) {
  NmeaParser::NmeaSentence result = NmeaParser::NMEA_NONE;

  for (const char *s = sentence; *s; ++s) {
    NmeaParser::NmeaSentence status = parser->processByte(*s); 
    if (status != NmeaParser::NMEA_NONE) {
      result = status;
    }
  }
  return result;
}

TEST(NmeaParserTest, TestVLW) {
  NmeaParser parser;

  sendSentence("$IIVLW,00430,N,002.3,N*55", &parser);

  EXPECT_EQ(430, parser.cumulativeWaterDistance());
  EXPECT_EQ(23, parser.waterDistance());
  EXPECT_EQ(1, parser.numSentences());
}

TEST(NmeaParserTest, TestMWV) {
  NmeaParser parser;
  EXPECT_EQ(NmeaParser::NMEA_AW,
           sendSentence("$IIMTW,020.0,C*21\n"
                        "$IIMTW,020.4,C*25\n"
                        "$IIMWV,010,R,004.8,N,A*2E", &parser));
  EXPECT_EQ(10, parser.awa().degrees());
  EXPECT_EQ(int(4.8f * 256.0f), int(256.0f * (float) parser.aws().knots()));
}

TEST(NmeaParserTest, TestRMC) {
  NmeaParser parser;

  EXPECT_EQ(
      NmeaParser::NMEA_TIME_POS,
      sendSentence(
          "$IIRMC,130222,A,4612.929,N,00610.063,E,01.5,286,100708,,,A*4E",
          &parser));

  EXPECT_EQ(46, parser.pos().lat.deg());
  EXPECT_EQ(12, parser.pos().lat.min());
  EXPECT_EQ(929, parser.pos().lat.mc());
  EXPECT_EQ(6, parser.pos().lon.deg());
  EXPECT_EQ(10, parser.pos().lon.min());
  EXPECT_EQ(63, parser.pos().lon.mc());

  EXPECT_EQ(1, parser.numSentences());

  sendSentence(
    "$GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62",
    &parser);

  EXPECT_EQ(-37, parser.pos().lat.deg());
  EXPECT_EQ(-51, parser.pos().lat.min());
  EXPECT_EQ(-650, parser.pos().lat.mc());
  EXPECT_EQ(145, parser.pos().lon.deg());
  EXPECT_EQ(7, parser.pos().lon.min());
  EXPECT_EQ(360, parser.pos().lon.mc());
  EXPECT_EQ(2, parser.numSentences());

  sendSentence(
    "$GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68",
    &parser);

  EXPECT_EQ(49, parser.pos().lat.deg());
  EXPECT_EQ(16, parser.pos().lat.min());
  EXPECT_EQ(450, parser.pos().lat.mc());
  EXPECT_EQ(-123, parser.pos().lon.deg());
  EXPECT_EQ(-11, parser.pos().lon.min());
  EXPECT_EQ(-120, parser.pos().lon.mc());
  EXPECT_EQ(3, parser.numSentences());

  sendSentence(
    "$GPRMC,084403,A,4951.4011,N,00746.5936,W,9.0,137.5,060814,4,W*64",
    &parser);

  EXPECT_EQ(49, parser.pos().lat.deg());
  EXPECT_EQ(51, parser.pos().lat.min());
  EXPECT_EQ(401, parser.pos().lat.mc());
  EXPECT_EQ(-7, parser.pos().lon.deg());
  EXPECT_EQ(-46, parser.pos().lon.min());
  EXPECT_EQ(-593, parser.pos().lon.mc());
  EXPECT_EQ(4, parser.numSentences());
}

TEST(NmeaParserTest, TestAccAngle) {
  AccAngle angle(3.2);
  EXPECT_NEAR(angle.toDouble(), 3.2, 1e-6);

  double asDegrees = -302.2865;
  angle = AccAngle(asDegrees);
  EXPECT_NEAR(angle.toDouble(), asDegrees, 1e-6);

  asDegrees = -300.12345;
  angle = AccAngle(asDegrees);
  EXPECT_NEAR(angle.toDouble(), asDegrees, 1e-6);

  angle.set(-45, -30, -123);

  asDegrees = - (45.0 + (30.0 + (123.0 / 1000.0)) / 60.0);
  EXPECT_NEAR(angle.toDouble(), asDegrees, 1e-6);
}

TEST(NmeaParserTest, AccAngleLizardLongitude) {
  AccAngle negative;

  // Longitude of Lizard point.
  negative.set(-5, -12, -425);
  EXPECT_NEAR(-5.207078, negative.toDouble(), 1e-5);
}

void testDist(const GeoPos &a, const GeoPos &b,
              double altitude, double dist) {
  GeoRef ref(a, altitude);
  ProjectedPos pa(ref, a);
  ProjectedPos pb(ref, b);
  EXPECT_NEAR(dist, pa.dist(pb), 1);
}

TEST(NmeaParserTest, TestGeoRef) {
  // The geographic coordinates and distances come from Google Earth. This
  // test verifies that distances computed match the ones computed by Google
  // Earth.

  GeoPos markA;
  markA.lat.set(54, 50, 422);
  markA.lon.set(9, 29, 885);

  GeoPos markB;
  markB.lat.set(54, 52, 58);
  markB.lon.set(9, 33, 726);

  GeoRef ref(markA, 0);

  ProjectedPos projA(ref, markA);

  EXPECT_NEAR(0, projA.x(), .01);
  EXPECT_NEAR(0, projA.y(), .01);

  ProjectedPos projB(ref, markB);

  EXPECT_NEAR(5109.1, projA.dist(projB), 3);

  testDist(
    GeoPos(46, 29, 624, 6, 36, 658),
    GeoPos(46, 30, 98, 6, 39, 677),
    372,
    3962.5);

  testDist(
    GeoPos(-42, 6, 31, -61, 35, 886),
    GeoPos(-42, 6, 957, -61, 35, 854),
    0,
    1715);
}

TEST(NmeaParserTest, TestGLL) {
  NmeaParser parser;

  EXPECT_EQ(
      NmeaParser::NMEA_GLL,
      sendSentence(
          "$IIGLL,4743.639,N,00322.305,W,084546,A,A*44",
          &parser));

  EXPECT_EQ(47, parser.pos().lat.deg());
  EXPECT_EQ(43, parser.pos().lat.min());
  EXPECT_EQ(639, parser.pos().lat.mc());
  EXPECT_EQ(-3, parser.pos().lon.deg());
  EXPECT_EQ(-22, parser.pos().lon.min());
  EXPECT_EQ(-305, parser.pos().lon.mc());

  EXPECT_EQ(8, parser.hour());
  EXPECT_EQ(45, parser.min());
  EXPECT_EQ(46, parser.sec());

  EXPECT_EQ(1, parser.numSentences());
}

TEST(NmeaParserTest, TestZDA) {
  NmeaParser parser;

  EXPECT_EQ(
      NmeaParser::NMEA_ZDA,
      sendSentence(
          "$IIZDA,084546,27,02,2015,,*55",
          &parser));
  EXPECT_EQ(8, parser.hour());
  EXPECT_EQ(45, parser.min());
  EXPECT_EQ(46, parser.sec());

  EXPECT_EQ(27, parser.day());
  EXPECT_EQ(2, parser.month());
  EXPECT_EQ(15, parser.year());

  EXPECT_EQ(1, parser.numSentences());
}

TEST(NmeaParserTest, TestVTG) {
  NmeaParser parser;

  EXPECT_EQ(
      NmeaParser::NMEA_VTG,
      sendSentence(
          "$IIVTG,316.,T,,M,06.2,N,11.5,K,A*2F",
          &parser));
  EXPECT_NEAR(316.0, (double)parser.gpsBearing().degrees(), 1e-3);
  EXPECT_NEAR(6.2, (double)parser.gpsSpeed().knots(), 1e-3);

  EXPECT_EQ(1, parser.numSentences());
}

TEST(NmeaParserTest, TestVWR) {
  NmeaParser parser;
  EXPECT_EQ(NmeaParser::NMEA_AW,
           sendSentence(
               "$IIVWR,037.,R,22.8,N,11.7,M,042.2,K*76",
               &parser));
  EXPECT_EQ(37, parser.awa().degrees());
  EXPECT_EQ(int(22.8f * 256.0f), int(256.0f * (float) parser.aws().knots()));

  EXPECT_EQ(NmeaParser::NMEA_AW,
           sendSentence(
               "$IIVWR,035.,L,24.4,N,12.6,M,045.2,K*65", &parser));
  EXPECT_EQ(-35, parser.awa().degrees());
  EXPECT_EQ(int(24.4f * 256.0f), int(256.0f * (float) parser.aws().knots()));

  EXPECT_EQ(2, parser.numSentences());
}

TEST(NmeaParserTest, TestVWT) {
  NmeaParser parser;
  EXPECT_EQ(NmeaParser::NMEA_TW,
           sendSentence(
               "$IIVWT,045.,L,19.6,N,10.1,M,036.3,K*68", &parser));
  EXPECT_EQ(-45, parser.twa().degrees());
  EXPECT_EQ(int(19.6f * 256.0f), int(256.0f * (float) parser.tws().knots()));

  EXPECT_EQ(NmeaParser::NMEA_TW,
           sendSentence(
               "$IIVWT,046.,R,18.9,N,09.7,M,035.0,K*75", &parser));
  EXPECT_EQ(46, parser.twa().degrees());
  EXPECT_EQ(int(18.9f * 256.0f), int(256.0f * (float) parser.tws().knots()));

  EXPECT_EQ(2, parser.numSentences());
}
