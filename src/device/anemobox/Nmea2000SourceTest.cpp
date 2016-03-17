#include <device/anemobox/Nmea2000Source.h>
#include <gtest/gtest.h>

using namespace sail;

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
  uint8_t data[7][8] = {
   { 0x60, 0x2b, 0x00, 0x74, 0x40, 0xc0, 0xca, 0x97 },
   { 0x61, 0x0a, 0x00, 0x10, 0xff, 0xed, 0xf5, 0x21 },
   { 0x62, 0xe5, 0x05, 0x00, 0xa9, 0xf1, 0x8f, 0xf6 },
   { 0x63, 0x88, 0x16, 0xf6, 0x90, 0xba, 0xc5, 0x03 },
   { 0x64, 0x00, 0x00, 0x00, 0x00, 0x12, 0xfc, 0x00 },
   { 0x65, 0xbe, 0x00, 0x54, 0x01, 0xe1, 0xf2, 0xff },
   { 0x66, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff }
  };

  for (int i = 0; i < 7; ++i) {
    source.process("test", 129029, data[i], 8, 83);
  }

  EXPECT_TRUE(dispatcher.get<DATE_TIME>()->dispatcher()->hasValue());
  TimeStamp val = dispatcher.val<DATE_TIME>();
  TimeStamp truth = TimeStamp::UTC(2015, 3, 6, 4, 56, 12);
  EXPECT_NEAR((truth - val).seconds(), 0, 1e-2);

  EXPECT_TRUE(dispatcher.get<GPS_POS>()->dispatcher()->hasValue());
}

TEST(Nmea2000SourceTest, SystemTime) {
  Dispatcher dispatcher;
  Nmea2000Source source(&dispatcher);
  
  // The following data gets parser by canboat analyser as:
  // System Time:  SID = 0; Source = GPS; Date = 2015.03.06; Time = 04:56:12
  //
  // Command line in canboat:
  // candump2analyzer  < samples/candumpSample2.txt | analyzer -raw
  const unsigned char data[] = { 0x00, 0xf0, 0x74, 0x40, 0xc0, 0xca, 0x97, 0x0a };
  source.process("test", 126992, data, sizeof(data), 82);

  EXPECT_TRUE(dispatcher.get<DATE_TIME>()->dispatcher()->hasValue());
  TimeStamp val = dispatcher.val<DATE_TIME>();
  TimeStamp truth = TimeStamp::UTC(2015, 3, 6, 4, 56, 12);
  EXPECT_NEAR((truth - val).seconds(), 0, 1e-2);
}

TEST(Nmea2000SourceTest, WindData) {
  Dispatcher dispatcher;
  Nmea2000Source source(&dispatcher);
  
  const unsigned char data[] = { 0x00, 0x34, 0x12, 0x50, 0x33, 0x03, 0x0, 0x0 };
  source.process("test", 130306, data, sizeof(data), 82);

  EXPECT_TRUE(dispatcher.get<TWA>()->dispatcher()->hasValue());
  EXPECT_NEAR(dispatcher.val<TWA>().degrees(), 75.3, .1);
}

