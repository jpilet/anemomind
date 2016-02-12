
#include <device/anemobox/FakeClockDispatcher.h>

#include <device/anemobox/logger/LogToNav.h>
#include <device/anemobox/logger/Logger.h>
#include <gtest/gtest.h>
#include <device/anemobox/logger/LogLoader.h>

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

TEST(LogToNavTest, ConvertNmea) {
  LogFile loggedData;
  makeLogFile(&loggedData);

  Array<Nav> converted = logFileToNavArray(loggedData);
  EXPECT_EQ(10, converted.size());

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(i, converted[i].geographicPosition().lon().degrees());
    EXPECT_EQ(i, converted[i].awa().degrees());
    EXPECT_EQ(i, converted[i].aws().knots());
  }
}

TEST(LogToNavTest, ConvertToDispatcher) {
  LogFile loggedData;
  makeLogFile(&loggedData);

  LogLoader loader;
  loader.load(loggedData);
  auto d = std::shared_ptr<Dispatcher>(new Dispatcher());
  loader.addToDispatcher(d.get());
  auto awaSamples = d->getSamples<AWA>();
  EXPECT_EQ(awaSamples.size(), 10);
  int counter = 0;
  for (auto sample: awaSamples.samples()) {
    EXPECT_NEAR(counter, sample.value.degrees(), 1.0e-3);
    counter++;
  }

}
