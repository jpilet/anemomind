/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/logimport/ProtobufLogLoader.h>
#include <device/anemobox/FakeClockDispatcher.h>
#include <device/anemobox/logger/Logger.h>

using namespace sail;

//TEST(ProtobufLogTest, LoadAFile) {
//  auto data = LogLoader::loadNavDataset(
//      PathBuilder::makeDirectory(Env::SOURCE_DIR)
//        .pushDirectory("datasets")
//        .pushDirectory("protobuflog").get());
//  EXPECT_LT(0, data.samples<AWA>().size());
//  EXPECT_LT(0, data.samples<GPS_POS>().size());
//  EXPECT_LT(0, data.samples<GPS_SPEED>().size());
//  EXPECT_LT(0, data.samples<GPS_BEARING>().size());
//}
//
//TEST(ProtobufLogTest, LoadRudderData) {
//  auto data = LogLoader::loadNavDataset(
//      PathBuilder::makeDirectory(Env::SOURCE_DIR)
//        .pushDirectory("datasets")
//        .pushDirectory("boat55dc89e6838caff0240960a9_rudder").get());
//  EXPECT_LT(12/*sufficiently large*/, data.samples<RUDDER_ANGLE>().size());
//}
//
//namespace {
//  auto diff = Duration<double>::hours(2.0);
//
//  TimeStamp sysTime(int i) {
//    return TimeStamp::UTC(2016, 6, 3, 10, 49, i);
//  }
//
//  TimeStamp trueGpsTime(int i) {
//    return sysTime(i) + diff;
//  }
//
//  TimeStamp gpsTimeWithOutliers(int i) {
//    if (i == 0 || i == 3 || i == 14 || i == 17 || i == 30 || i == 31) {
//      return TimeStamp::UTC(2016, 6, 3, 19, 34, 0);
//    }
//    return trueGpsTime(i);
//  }
//
//}
//
//TEST(LoggerTest, LogTime) {
//  FakeClockDispatcher dispatcher;
//  Logger logger(&dispatcher);
//
//
//  const int n = 60;
//  for (int i = 0; i < n; i++) {
//    dispatcher.setTime(sysTime(i));
//    dispatcher.publishValue(DATE_TIME, "Test", gpsTimeWithOutliers(i));
//    dispatcher.publishValue(AWS, "Test", Velocity<double>::metersPerSecond(0.4*i));
//  }
//
//  LogFile saved;
//  logger.flushTo(&saved);
//
//  LogLoader loader;
//  loader.load(saved);
//  auto ds = loader.makeNavDataset();
//
//  auto aws = ds.samples<AWS>();
//  EXPECT_EQ(aws.size(), n);
//  for (int i = 0; i < n; i++) {
//    auto x = aws[i];
//    EXPECT_NEAR((x.time - trueGpsTime(i)).seconds(), 0.0, 1.0e-6);
//    EXPECT_NEAR(aws[i].value.metersPerSecond(), 0.4*i, 0.01);
//  }
//}

namespace {
  TimeStamp indexToTime(int index) {
    return TimeStamp::UTC(2016, 7, 13, 19, 22, 0) + double(index)*Duration<double>::seconds(2.4);
  }
}


TEST(LoggerTest, RegularizeTimeInPlace) {
  auto s = Duration<double>::seconds(1.0);


  int n = 12;
  std::vector<TimeStamp> times;
  for (int i = 0; i < n; i++) {
    times.push_back(indexToTime(i));
  }

  // Bad times:
  times[0] = TimeStamp::UTC(2070, 5, 4, 19, 22, 0);
  times[4] = TimeStamp::UTC(2048, 5, 4, 19, 22, 0);

  ProtobufLogLoader::regularizeTimesInPlace(&times);

  for (int i = 0; i < n; i++) {
    auto diff = times[i] - indexToTime(i);
    EXPECT_NEAR(diff.seconds(), 0.0, 0.001);
  }
}

