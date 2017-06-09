#include <device/anemobox/logger/Logger.h>

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <device/anemobox/FakeClockDispatcher.h>
#include <cstdio>
#include <fstream>

using namespace sail;

TEST(LoggerTest, SmokeTest) {
  Dispatcher dispatcher;
  Logger logger(&dispatcher);

  double values[] = {1, 2, 3, 7, 8, 3, 4, 5 };
  const int num_values = sizeof(values)/sizeof(values[0]);

  for (int i = 0; i < num_values; ++i) {
    dispatcher.publishValue(AWA, "test", Angle<double>::degrees(values[i]));
  }
  const char filename[] = "./00000000576A5F3F.log";
  EXPECT_TRUE(logger.flushAndSaveToFile(filename));

  LogFile loaded;
  EXPECT_TRUE(Logger::read(filename, &loaded));
  EXPECT_EQ(1, loaded.stream_size());

  EXPECT_EQ(num_values, loaded.stream(0).timestampssinceboot_size());
  EXPECT_EQ(num_values, loaded.stream(0).angles().deltaangle_size());

  std::vector<Angle<double>> angles;
  Logger::unpack(loaded.stream(0).angles(), &angles);
  EXPECT_EQ(num_values, angles.size());

  for (int i = 0; i < num_values; ++i) {
    EXPECT_NEAR(values[i], angles[i].degrees(), 1.0/100.0);
  }
  boost::filesystem::remove(filename);
}

TEST(LoggerTest, LogText) {
  Dispatcher dispatcher;
  Logger logger(&dispatcher);

  logger.logText("source A", "sentence A1");
  logger.logText("source B", "sentence B1");
  logger.logText("source A", "sentence A2");

  LogFile data;
  logger.flushTo(&data);

  EXPECT_FALSE(data.has_bootcount());

  EXPECT_EQ(2, data.text_size());

  for (int i = 0; i < 2; ++i) {
    auto stream = data.text(i);
    if (stream.source() == "source A") {
      EXPECT_EQ(2, stream.text_size());
      EXPECT_EQ(2, stream.timestampssinceboot_size());
      EXPECT_EQ("sentence A1", stream.text(0));
      EXPECT_EQ("sentence A2", stream.text(1));
    } else {
      EXPECT_EQ("source B", stream.source());
      EXPECT_EQ(1, stream.text_size());
      EXPECT_EQ(1, stream.timestampssinceboot_size());
      EXPECT_EQ("sentence B1", stream.text(0));
    }
  }

  // logger has been flushed. Flushing a 2nd time should give an empty result.
  LogFile empty;
  logger.flushTo(&empty);
  EXPECT_EQ(0, empty.text_size());
}

TEST(LoggerTest, MultipleSources) {
  Dispatcher dispatcher;
  Logger logger(&dispatcher);

  EXPECT_EQ(0, logger.numListeners());
  dispatcher.publishValue(AWA, "source1", Angle<double>::degrees(1));
  EXPECT_EQ(1, logger.numListeners());
  dispatcher.publishValue(AWA, "source2", Angle<double>::degrees(2));
  EXPECT_EQ(2, logger.numListeners());
  dispatcher.publishValue(AWA, "source3", Angle<double>::degrees(3));
  EXPECT_EQ(3, logger.numListeners());

  // Source 1 already exists: it should not create a new listener.
  dispatcher.publishValue(AWA, "source1", Angle<double>::degrees(4));
  EXPECT_EQ(3, logger.numListeners());

  LogFile loaded;
  logger.flushTo(&loaded);
  EXPECT_EQ(3, loaded.stream_size());

  for (int i = 0; i < 3; ++i) {
    std::vector<Angle<double>> angles;
    Logger::unpack(loaded.stream(i).angles(), &angles);
    EXPECT_EQ(i == 0 ? 2 : 1, angles.size());
    EXPECT_NEAR(i + 1, angles[0].degrees(), 1e-3);
  }
  EXPECT_EQ("source1", loaded.stream(0).source());
  EXPECT_EQ("source2", loaded.stream(1).source());
  EXPECT_EQ("source3", loaded.stream(2).source());
}

TEST(LoggerTest, LogOrientation) {
  Dispatcher dispatcher;
  Logger logger(&dispatcher);

  AbsoluteOrientation orient;
  orient.heading = Angle<>::degrees(1);
  orient.roll = Angle<>::degrees(2);
  orient.pitch = Angle<>::degrees(3);
  dispatcher.publishValue(ORIENT, "test", orient);

  orient.heading = Angle<>::degrees(4);
  orient.roll = Angle<>::degrees(5);
  orient.pitch = Angle<>::degrees(6);
  dispatcher.publishValue(ORIENT, "test", orient);

  LogFile saved;
  logger.flushTo(&saved);

  std::vector<AbsoluteOrientation> unpacked;
  Logger::unpack(saved.stream(0).orient(), &unpacked);
  EXPECT_EQ(2, unpacked.size());
  EXPECT_EQ(1, unpacked[0].heading.degrees());
  EXPECT_EQ(2, unpacked[0].roll.degrees());
  EXPECT_EQ(3, unpacked[0].pitch.degrees());
  EXPECT_EQ(4, unpacked[1].heading.degrees());
  EXPECT_EQ(5, unpacked[1].roll.degrees());
  EXPECT_EQ(6, unpacked[1].pitch.degrees());
}

TEST(LoggerTest, LogTime) {
  FakeClockDispatcher dispatcher;
  Logger logger(&dispatcher);

  auto laterInTheMorning = TimeStamp::UTC(2016, 5, 27, 8, 15, 0);
  auto inTheMorning = TimeStamp::UTC(2016, 5, 27, 8, 0, 0);

  dispatcher.setTime(inTheMorning);

  auto today = TimeStamp::UTC(2016, 5, 27, 14, 6, 0);
  auto stillToday = TimeStamp::UTC(2016, 5, 27, 14, 7, 0);

  AbsoluteOrientation orient;
  dispatcher.publishValue(DATE_TIME, "test", today);
  dispatcher.setTime(laterInTheMorning);
  dispatcher.publishValue(DATE_TIME, "test", stillToday);

  LogFile saved;
  logger.flushTo(&saved);

  std::vector<TimeStamp> externalTimes;
  Logger::unpack(saved.stream(0).exttimes(), &externalTimes);
  EXPECT_EQ(2, externalTimes.size());
  EXPECT_EQ(today, externalTimes[0]);
  EXPECT_EQ(stillToday, externalTimes[1]);

  std::vector<TimeStamp> systemTimes;
  Logger::unpackTime(saved.stream(0), &systemTimes);
  EXPECT_EQ(2, systemTimes.size());
  EXPECT_TRUE(systemTimes[0].defined());
  EXPECT_EQ(systemTimes[0], inTheMorning);
  EXPECT_TRUE(systemTimes[1].defined());
  EXPECT_EQ(systemTimes[1], laterInTheMorning);
  EXPECT_EQ(saved.stream(0).shortname(), "dateTime");
}

TEST(LoggerTest, ReadInteger0) {
  const char filename[] = "/tmp/this_file_should_not_exist.txt";
  std::remove(filename);
  auto value = readIntegerFromTextFile(filename);
  EXPECT_FALSE(value.defined());
}

TEST(LoggerTest, ReadInteger1) {
  const char filename[] = "/tmp/here_there_is_an_integer_i_hope.txt";
  {
    std::ofstream file(filename, std::ios_base::trunc);
    file << 119;
  }
  auto value = readIntegerFromTextFile(filename);
  EXPECT_TRUE(value.defined());
  EXPECT_EQ(value.get(), 119);
  std::remove(filename);
}

TEST(LoggerTest, ReadInteger2) {
  const char filename[] = "/tmp/here_there_is_an_integer_i_hope_with_whitespace.txt";
  {
    std::ofstream file(filename, std::ios_base::trunc);
    file << "  " << 119 << "\n ";
  }
  auto value = readIntegerFromTextFile(filename);
  EXPECT_TRUE(value.defined());
  EXPECT_EQ(value.get(), 119);
  std::remove(filename);
}

TEST(LoggerTest, LogBinary) {
  FakeClockDispatcher dispatcher;
  Logger logger(&dispatcher);

  TimeStamp today = TimeStamp::UTC(2016, 5, 27, 14, 6, 0);
  TimeStamp tomorrow = today + Duration<>::days(1);
  dispatcher.setTime(today);
  dispatcher.publishValue(VALID_GPS, "test", BinaryEdge::ToOn);
  dispatcher.setTime(tomorrow);
  dispatcher.publishValue(VALID_GPS, "test", BinaryEdge::ToOff);

  LogFile saved;
  logger.flushTo(&saved);

  EXPECT_EQ(1, saved.stream_size());
  EXPECT_EQ(2, saved.stream(0).binary().edges_size());
}

const ::sail::Nmea2000Sentences& findNmea2000Sentences(
    const LogFile& src,
    int64_t id, Nmea2000SizeClass sc) {
  for (int i = 0; i < src.rawnmea2000_size(); i++) {
    const ::sail::Nmea2000Sentences& x = src.rawnmea2000(i);
    if (x.sentence_id() == id && ((x.oddsizesentences_size() == 0)
            == (sc == Nmea2000SizeClass::Regular))) {
      return x;
    }
  }
  EXPECT_TRUE(false);
  return src.rawnmea2000(0);
}

TEST(LoggerTest, LogNmea) {
  auto time = TimeStamp::UTC(2016, 6, 8, 15, 19, 0);

  LogFile saved0, saved1;
  {
    FakeClockDispatcher dispatcher;
    Logger logger(&dispatcher);
    dispatcher.setTime(time);
    logger.logRawNmea2000(34200, 119, 3, "abc");
    dispatcher.setTime(time + 1.0_s);
    logger.logRawNmea2000(36200, 119, 3, "def");
    logger.logRawNmea2000(36250, 17, 3, "xyz");
    logger.logRawNmea2000(36250, 17, 8, "abcdefgh");
    logger.flushTo(&saved0);

    logger.logRawNmea2000(36600, 119, 3, "ghi");
    logger.flushTo(&saved1);
  }

  EXPECT_EQ(saved0.rawnmea2000_size(), 3);
  std::vector<TimeStamp> times;
  const auto& odd = findNmea2000Sentences(saved0, 119,
      Nmea2000SizeClass::Odd);
  unpackTimeStamps(
      odd.timestampssinceboot(), &times);
  EXPECT_EQ(odd.oddsizesentences(1), "def");
  EXPECT_EQ(odd.sentence_id(), 119);
  EXPECT_EQ(times.size(), 2);
  EXPECT_EQ(times[0].toMilliSecondsSince1970(), 34200);
  EXPECT_EQ(times[1].toMilliSecondsSince1970(), 36200);

  EXPECT_EQ(saved1.rawnmea2000_size(), 1);
}
