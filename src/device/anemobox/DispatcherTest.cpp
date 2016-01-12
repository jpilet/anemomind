#include <device/anemobox/Dispatcher.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace sail;

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

