
#include <device/anemobox/FakeClockDispatcher.h>

#include <device/anemobox/logger/Logger.h>
#include <gtest/gtest.h>
#include <server/nautical/logimport/LogLoader.h>

#include <server/common/Env.h>

using namespace sail;
using std::string;

namespace {

void sendFakeValue(int val, FakeClockDispatcher *dispatcher) {
  dispatcher->publishValue(GPS_POS, "test",
                           GeographicPosition<double>(Angle<double>::degrees(val),
                                                      Angle<double>::degrees(val)));
  dispatcher->publishValue(AWA, "test", Angle<double>::degrees(val));
  dispatcher->publishValue(AWS, "test", Velocity<double>::knots(val));
}

void makeLogFile(LogFile *loggedData) {
  FakeClockDispatcher dispatcher;
  Logger logger(&dispatcher);

  for (int i = 0 ; i < 10; ++i) {
    sendFakeValue(i, &dispatcher);
    dispatcher.advance(Duration<>::seconds(1));
  }
  logger.flushTo(loggedData);
}


}  // namespace

TEST(LogToNavTest, ConvertToDispatcher) {
  LogFile loggedData;
  makeLogFile(&loggedData);

  LogLoader loader;
  loader.load(loggedData);
  auto d = std::shared_ptr<Dispatcher>(new Dispatcher());
  loader.addToDispatcher(d.get());
  auto awa = d->values<AWA>();
  EXPECT_EQ(awa.size(), 10);
  int counter = 0;
  for (auto sample: awa.samples()) {
    EXPECT_NEAR(counter, sample.value.degrees(), 1.0e-3);
    counter++;
  }

}

TEST(LogLoaderTest, DecompressTest) {
  LogLoader loader;
  EXPECT_TRUE(loader.loadFile(std::string(Env::SOURCE_DIR) + "/datasets/tinylog.txt.gz"));
}

