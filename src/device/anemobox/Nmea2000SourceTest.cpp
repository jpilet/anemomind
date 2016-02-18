#include <device/anemobox/Nmea2000Source.h>
#include <gtest/gtest.h>

using namespace sail;

#if 0
   This test is currently disabled because long Pgns such as
   GnssPositionData are not handled properly.

TEST(Nmea2000SourceTest, GnssPositionData) {
  Dispatcher dispatcher;
  Nmea2000Source source(&dispatcher);

  // The following data gets parser by canboat analyser as:
  // 2016-02-18-13:25:17.000 3 127 255 129029
  // GNSS Position Data:
  //   SID = 0;
  //   Date = 2015.03.06;
  //   Time = 04:56:12;
  //   Latitude = 42.4783080;
  //   Longitude = -71.4232898;
  //   Altitude = Unhandled value 63290000 (63290001);
  //   GNSS type = GPS+GLONASS;
  //   Method = GNSS fix;
  //   Integrity = No integrity checking;
  //   Number of SVs = 0; HDOP = 1.90; PDOP = 3.40;
  //   Geoidal Separation = -33.59 m; Reference Stations = Unknown;
  //   Reference Station Type = Unknown; Reference Station ID = 15
  //
127 255 129029 : 60 2b 00 74 40 c0 ca 97
127 255 129029 : 61 0a 00 10 ff ed f5 21
127 255 129029 : 62 e5 05 00 a9 f1 8f f6
127 255 129029 : 63 88 16 f6 90 ba c5 03
127 255 129029 : 64 00 00 00 00 12 fc 00
127 255 129029 : 65 be 00 54 01 e1 f2 ff
}
#endif

TEST(Nmea2000SourceTest, SystemTime) {
  Dispatcher dispatcher;
  Nmea2000Source source(&dispatcher);
  
  // The following data gets parser by canboat analyser as:
  // System Time:  SID = 0; Source = GPS; Date = 2015.03.06; Time = 04:56:12
  //
  // Command line in canboat:
  // candump2analyzer  < samples/candumpSample2.txt | analyzer -raw
  const unsigned char data[] = { 0x00, 0xf0, 0x74, 0x40, 0xc0, 0xca, 0x97, 0x0a };
  source.process("test", 126992, data, sizeof(data));

  EXPECT_TRUE(dispatcher.get<DATE_TIME>()->dispatcher()->hasValue());
  TimeStamp val = dispatcher.val<DATE_TIME>();
  TimeStamp truth = TimeStamp::UTC(2015, 3, 6, 4, 56, 12);
  EXPECT_NEAR((truth - val).seconds(), 0, 1e-2);
}
