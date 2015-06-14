#include <device/anemobox/logger/Logger.h>

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

using namespace sail;

TEST(LoggerTest, SmokeTest) {
  Dispatcher dispatcher;
  Logger logger(&dispatcher);
  logger.subscribe();

  double values[] = {1, 2, 3, 7, 8, 3, 4, 5 };
  const int num_values = sizeof(values)/sizeof(values[0]);

  for (int i = 0; i < num_values; ++i) {
    dispatcher.awa()->publishValue("test", Angle<double>::degrees(values[i]));
  }
  std::string filename;
  EXPECT_TRUE(logger.flushAndSave("./", &filename));

  LogFile loaded;
  EXPECT_TRUE(Logger::read(filename, &loaded));
  EXPECT_EQ(1, loaded.stream_size());

  EXPECT_EQ(num_values, loaded.stream(0).timestamps_size());
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

  EXPECT_EQ(2, data.text_size());

  for (int i = 0; i < 2; ++i) {
    auto stream = data.text(i);
    if (stream.shortname() == "source A") {
      EXPECT_EQ(2, stream.text_size());
      EXPECT_EQ(2, stream.timestamps_size());
      EXPECT_EQ("sentence A1", stream.text(0));
      EXPECT_EQ("sentence A2", stream.text(1));
    } else {
      EXPECT_EQ("source B", stream.shortname());
      EXPECT_EQ(1, stream.text_size());
      EXPECT_EQ(1, stream.timestamps_size());
      EXPECT_EQ("sentence B1", stream.text(0));
    }
  }

  // logger has been flushed. Flushing a 2nd time should give an empty result.
  LogFile empty;
  logger.flushTo(&empty);
  EXPECT_EQ(0, empty.text_size());
}
