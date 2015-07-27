
#include <device/anemobox/FakeClockDispatcher.h>

#include <device/anemobox/logger/LogToNav.h>
#include <device/anemobox/logger/Logger.h>
#include <gtest/gtest.h>

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

}  // namespace

TEST(LogToNavTest, ConvertNmea) {
  FakeClockDispatcher dispatcher;
  Logger logger(&dispatcher);

  for (int i = 0 ; i < 10; ++i) {
    sendFakeValue(i, &dispatcher);
    dispatcher.advance(Duration<>::seconds(1));
  }

  LogFile loggedData;
  logger.flushTo(&loggedData);

  Array<Nav> converted = logFileToNavArray(loggedData);
  EXPECT_EQ(10, converted.size());

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(i, converted[i].geographicPosition().lon().degrees());
    EXPECT_EQ(i, converted[i].awa().degrees());
    EXPECT_EQ(i, converted[i].aws().knots());
  }
}
