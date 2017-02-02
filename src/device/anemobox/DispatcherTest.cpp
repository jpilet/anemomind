#include <device/anemobox/Dispatcher.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>

using namespace sail;

using ::testing::ResultOf;
using ::testing::DoubleEq;

TEST(DispatcherTest, SingleValTest) {
  Dispatcher dispatcher;
  EXPECT_FALSE(dispatcher.get<AWA>()->dispatcher()->hasValue());
  dispatcher.publishValue(AWA, "test", Angle<>::degrees(7));
  EXPECT_TRUE(dispatcher.get<AWA>()->dispatcher()->hasValue());
  EXPECT_NEAR(7, dispatcher.val<AWA>().degrees(), 1e-6);
}

TEST(DispatcherTest, PriorityTest) {
  Dispatcher dispatcher;
  dispatcher.setSourcePriority("low", 0);
  dispatcher.setSourcePriority("high", 10);

  EXPECT_EQ(0, dispatcher.sourcePriority("low"));
  EXPECT_EQ(10, dispatcher.sourcePriority("high"));

  dispatcher.publishValue(AWA, "low", Angle<>::degrees(1));

  EXPECT_EQ("low", dispatcher.get<AWA>()->source());
  dispatcher.publishValue(AWA, "high", Angle<>::degrees(2));
  EXPECT_EQ("high", dispatcher.get<AWA>()->source());
  dispatcher.publishValue(AWA, "low", Angle<>::degrees(3));
  EXPECT_EQ("high", dispatcher.get<AWA>()->source());
  dispatcher.publishValue(AWA, "high", Angle<>::degrees(4));
  dispatcher.publishValue(AWA, "low", Angle<>::degrees(5));
  dispatcher.publishValue(AWA, "high", Angle<>::degrees(6));
  dispatcher.publishValue(AWA, "low", Angle<>::degrees(7));
  EXPECT_EQ(3, dispatcher.get<AWA>()->dispatcher()->values().size());
  EXPECT_NEAR(6, dispatcher.val<AWA>().degrees(), 1e-6);
}

TEST(DispatcherTest, FreshTest) {
  Dispatcher dispatcher;

  dispatcher.publishValue(AWA, "low", Angle<>::degrees(1));

  auto instant = Duration<>::milliseconds(5);
  EXPECT_TRUE(dispatcher.get<AWA>()->isFresh(instant));
  sleep(instant);
  EXPECT_FALSE(dispatcher.get<AWA>()->isFresh(instant));
}

TEST(DispatcherTest, InsertValues) {
  Dispatcher dispatcher;

  TimedSampleCollection<Angle<double>>::TimedVector values;
  values.push_back(TimedValue<Angle<>>(TimeStamp::now(),
                                       Angle<>::degrees(17)));
  values.push_back(TimedValue<Angle<>>(TimeStamp::now() + Duration<>::seconds(1),
                                       Angle<>::degrees(18)));
  values.push_back(TimedValue<Angle<>>(TimeStamp::now() + Duration<>::seconds(2),
                                       Angle<>::degrees(16)));

  dispatcher.insertValues<Angle<double>>(AWA, "batch source", values);

  EXPECT_EQ(3, dispatcher.get<AWA>()->dispatcher()->values().size());
  EXPECT_NEAR(16, dispatcher.val<AWA>().degrees(), 1e-6);
}

class MockListener : public Listener<Angle<>> {
public:
    MOCK_METHOD1(onNewValue, void(const ValueDispatcher<Angle<>> &));
};

double degrees(const ValueDispatcher<Angle<>>& d) {
    EXPECT_TRUE(d.hasValue());
    return d.lastValue().degrees();
}

TEST(DispatcherTest, Publish) {
    Dispatcher dispatcher;
    MockListener listener;
    dispatcher.get<AWA>()->dispatcher()->subscribe(&listener);
    
    EXPECT_CALL(listener, onNewValue(ResultOf(degrees, DoubleEq(7))));
    dispatcher.publishValue(AWA, "test", Angle<>::degrees(7));
    
    // Different channel, should not call onNewValue
    dispatcher.publishValue(GPS_BEARING, "test", Angle<>::degrees(3));

    // Second value on AWA channel, should call onNewValue
    EXPECT_CALL(listener, onNewValue(ResultOf(degrees, DoubleEq(9))));
    dispatcher.publishValue(AWA, "test", Angle<>::degrees(9));
}

TEST(DispatcherTest, TestValueFromSourceAt) {
  Dispatcher dispatcher;

  TimeStamp base = TimeStamp::now();
  TimedSampleCollection<Angle<double>>::TimedVector values;

  values.push_back(TimedValue<Angle<>>(
          base + Duration<>::seconds(0), Angle<>::degrees(17)));
  values.push_back(TimedValue<Angle<>>(
          base + Duration<>::seconds(1), Angle<>::degrees(18)));
  values.push_back(TimedValue<Angle<>>(
          base + Duration<>::seconds(2), Angle<>::degrees(19)));

  // 98 sec Gap
  values.push_back(TimedValue<Angle<>>(
          base + Duration<>::seconds(100), Angle<>::degrees(3)));
  values.push_back(TimedValue<Angle<>>(
          base + Duration<>::seconds(101), Angle<>::degrees(2)));
                                       
  dispatcher.insertValues<Angle<double>>(AWA, "source", values);

  Duration<> limit = Duration<>::seconds(2);
  Optional<Angle<>> r =
    dispatcher.valueFromSourceAt<AWA>("source", base + Duration<>::seconds(1.3), limit);
  EXPECT_TRUE(r.defined());
  EXPECT_NEAR(r.get().degrees(), 18, 1e-5);

  // try to lookup within the gap
  r = dispatcher.valueFromSourceAt<AWA>("source", base + Duration<>::seconds(60), limit);
  EXPECT_FALSE(r.defined());

  // lookup after the gap
  r = dispatcher.valueFromSourceAt<AWA>("source", base + Duration<>::seconds(100), limit);
  EXPECT_TRUE(r.defined());
  EXPECT_NEAR(r.get().degrees(), 3, 1e-5);
}

TEST(DispatcherTest, sourcesForChannelTest) {
  Dispatcher dispatcher;
  EXPECT_EQ(0, dispatcher.sourcesForChannel(AWA).size());
  dispatcher.publishValue(AWA, "source 1", Angle<>::degrees(1));
  dispatcher.publishValue(AWA, "source 2", Angle<>::degrees(2));
  EXPECT_EQ((std::vector<std::string>{"source 1", "source 2"}),
            dispatcher.sourcesForChannel(AWA));
}

TEST(DispatcherTest, hasSourceTest) {
  Dispatcher dispatcher;
  EXPECT_FALSE(dispatcher.hasSource(AWA, "source 1"));
  EXPECT_FALSE(dispatcher.hasSource(AWA, "source 2"));

  dispatcher.publishValue(AWA, "source 1", Angle<>::degrees(1));
  EXPECT_TRUE(dispatcher.hasSource(AWA, "source 1"));
  EXPECT_FALSE(dispatcher.hasSource(AWS, "source 1"));

  dispatcher.publishValue(AWA, "source 2", Angle<>::degrees(2));
  EXPECT_TRUE(dispatcher.hasSource(AWA, "source 2"));

  EXPECT_FALSE(dispatcher.hasSource(AWA, "source 3"));
}
