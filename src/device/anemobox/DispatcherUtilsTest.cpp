/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <device/anemobox/DispatcherUtils.h>

using namespace sail;

namespace {
  auto offset = sail::TimeStamp::UTC(2016, 02, 26, 16, 4, 0);
  auto s = Duration<double>::seconds(1.0);
  auto kn = Velocity<double>::knots(1.0);
}

TEST(DispatcherUtilsTest, Merging) {

  const DataCode Code = AWS;
  typedef TypeForCode<Code>::type T;
  typedef TimedSampleCollection<T>::TimedVector TimedVector;

  {
    TimedVector A{
      {offset + 0.3*s,    3.0*kn},
      {offset + 0.6*s,    4.0*kn},
      {offset + 1233.0*s, 13.0*kn}
    };
    TimedVector B{
      {offset + 0.4*s,    17.0*kn}
    };

    std::map<std::string, std::shared_ptr<DispatchData>> sources{
      {"A", makeDispatchDataFromSamples<Code>("A", A)},
      {"B", makeDispatchDataFromSamples<Code>("B", B)},
    };

    std::map<std::string, int> priorities{
      {"A", 1},
      {"B", 2}
    };

    auto merged = mergeChannels(Code, "C", priorities, sources);
    auto values = toTypedDispatchData<Code>(merged.get())->dispatcher()->values().samples();
    EXPECT_EQ(values.size(), 2);
    EXPECT_NEAR((values[0].time - offset).seconds(), 0.4, 1.0e-6);
    EXPECT_NEAR(values[0].value.knots(), 17.0, 1.0e-6);
    EXPECT_NEAR((values[1].time - offset).seconds(), 1233.0, 1.0e-6);
    EXPECT_NEAR(values[1].value.knots(), 13.0, 1.0e-6);
  }
}




namespace {
  std::shared_ptr<DispatchData> ptr(unsigned long tag) {
    return std::shared_ptr<DispatchData>((DispatchData *)tag,
        [](DispatchData *x) {});
  }

  std::shared_ptr<DispatchData> get(const std::map<DataCode, std::map<std::string,
      std::shared_ptr<DispatchData>>> &src, DataCode c,
      const std::string &s) {
    auto x = src.find(c);
    if (x != src.end()) {
      auto y = x->second.find(s);
      if (y != x->second.end()) {
        return y->second;
      }
    }
    return std::shared_ptr<DispatchData>();
  }
}

TEST(DispatcherUtilsTest, MergeDispatchDataMapsTest) {
  EXPECT_EQ(ptr(0xBEEF), ptr(0xBEEF));
  EXPECT_NE(ptr(0xBEEF), ptr(0xDEAD));


  std::map<DataCode, std::map<std::string,
            std::shared_ptr<DispatchData>>> A{
    {GPS_POS, {{"a", ptr(0xDEADBEEF)}}},
    {GPS_SPEED, {{"b", ptr(0xDEAD)}}}
  };

  std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> B{
    {GPS_POS, {{"a", ptr(0xBEEF)}}},
    {GPS_BEARING, {{"c", ptr(0xF00D)}}}
  };

  auto C = mergeDispatchDataMaps(A, B);
  EXPECT_EQ(ptr(0xBEEF), get(C, GPS_POS, "a"));
  EXPECT_EQ(ptr(0xDEAD), get(C, GPS_SPEED, "b"));
  EXPECT_EQ(ptr(0xF00D), get(C, GPS_BEARING, "c"));


  {
    auto difs = listDataCodesWithDifferences(A, B);
    EXPECT_EQ(difs.size(), 3);
    EXPECT_EQ(difs.count(GPS_POS), 1);
    EXPECT_EQ(difs.count(GPS_SPEED), 1);
    EXPECT_EQ(difs.count(GPS_BEARING), 1);
  }{
    auto difs = listDataCodesWithDifferences(A, A);
    EXPECT_EQ(difs.size(), 0);
  }{
    auto difs = listDataCodesWithDifferences(A, C);
    EXPECT_EQ(difs.size(), 2);
    EXPECT_EQ(difs.count(GPS_POS), 1);
    EXPECT_EQ(difs.count(GPS_BEARING), 1);
  }{
    auto difs = listDataCodesWithDifferences(B, C);
    EXPECT_EQ(difs.size(), 1);
    EXPECT_EQ(difs.count(GPS_SPEED), 1);
  }
}

TEST(DisaptcherUtilsTest, TestSomeMore) {
  const DataCode Code = AWS;
  typedef TypeForCode<Code>::type T;
  typedef TimedSampleCollection<T>::TimedVector TimedVector;

  TimedVector A{
    {offset + 0.3*s,    3.0*kn},
    {offset + 0.6*s,    4.0*kn},
    {offset + 1233.0*s, 13.0*kn}
  };
  auto a = makeDispatchDataFromSamples<Code>("A", A);

  TimedVector B{
    {offset + 0.4*s,    17.0*kn}
  };
  auto b = makeDispatchDataFromSamples<Code>("B", B);

  TimedVector A2{
    {offset + 0.4*s,    17.0*kn}
  };
  auto a2 = makeDispatchDataFromSamples<Code>("A", A2);

  auto d0 = std::make_shared<Dispatcher>();
  EXPECT_EQ(0, d0->allSources().size());
  std::shared_ptr<Dispatcher> d1 = mergeDispatcherWithDispatchDataMap(d0.get(), {
       {Code, {{"A", a},
               {"B", b}}
       }
    });
  d1->setSourcePriority("A", 119);

  EXPECT_EQ(0, d0->allSources().size());
  EXPECT_EQ(2, countChannels(d1.get()));
  EXPECT_EQ(a, d1->dispatchDataForSource(Code, "A"));
  EXPECT_EQ(b, d1->dispatchDataForSource(Code, "B"));
  std::shared_ptr<Dispatcher> d2 = mergeDispatcherWithDispatchDataMap(d1.get(), {
       {Code, {{"A", a2}}},
    });
  EXPECT_EQ(d2->sourcePriority("A"), 119);

  EXPECT_EQ(a, d1->dispatchDataForSource(Code, "A"));
  EXPECT_EQ(b, d1->dispatchDataForSource(Code, "B"));

  EXPECT_EQ(a2, d2->dispatchDataForSource(Code, "A"));
  EXPECT_EQ(b, d1->dispatchDataForSource(Code, "B"));
}

std::shared_ptr<TypedDispatchData<Angle<double>>> getAwaData(
    const Dispatcher* d) {
  return std::static_pointer_cast<
        TypedDispatchData<Angle<double>>>(d->allSources().at(AWA).at("src"));

}

TEST(DispatcherUtilsTest, Replay) {
  Dispatcher d;
  auto offset = TimeStamp::UTC(2016, 3, 19, 14, 19, 0);
  auto seconds = Duration<double>::seconds(1.0);
  auto degrees = Angle<double>::degrees(1.0);

  typedef Angle<double> T;

  TimedSampleCollection<T>::TimedVector values;
  for (int i = 0; i < 9; i++) {
    double x = i;
    values.push_back(TimedValue<T>(offset + x*seconds, x*degrees));
  }

  d.insertValues<T>(AWA, "src", values);

  class AwaListener : public Listener<Angle<double> > {
  public:
    AwaListener(Dispatcher *dst) : _dst(dst), _counter(0) {}

    void onNewValue(const ValueDispatcher<Angle<double> > &dispatcher) {
      _counter++;
      auto data = toTypedDispatchData<AWA>(_dst->allSources()
                                           .find(AWA)->second.find("src")->second.get())
        ->dispatcher()->values().samples();

      EXPECT_EQ(_counter, data.size());
      _dst->publishValue<T>(AWA, "dst", 3.0*data.back().value);
    }

    Dispatcher *_dst;
    int _counter;
  };

  ReplayDispatcher d2;
  AwaListener awaListener(&d2);
  d2.get<AWA>()->dispatcher()->subscribe(&awaListener);
  d2.replay(&d);

  // Check that we did not duplicate identical data.
  EXPECT_EQ(getAwaData(&d)->dispatcher(),
           getAwaData(&d2)->dispatcher());

  EXPECT_EQ(9, awaListener._counter);

  auto awaSrc = toTypedDispatchData<AWA>(
      d2.allSources().find(AWA)->second.find("src")
      ->second.get())->dispatcher()->values().samples();
  auto awaDst = toTypedDispatchData<AWA>(
      d2.allSources().find(AWA)->second.find("dst")
      ->second.get())->dispatcher()->values().samples();

  EXPECT_EQ(awaSrc.size(), awaDst.size());
  for (int i = 0; i < awaSrc.size(); i++) {
    auto x = awaSrc[i];
    auto y = awaDst[i];
    EXPECT_EQ(x.time, y.time);
    EXPECT_NEAR(x.value.degrees()*3, y.value.degrees(), 0.001);
  }

  EXPECT_EQ(countChannels(&d2), 2);

  auto d3 = filterChannels(&d2, [&](DataCode c, const std::string &src) {
    return src == "src";
    }, true);
  
  EXPECT_EQ(countChannels(&d2), 2);
  EXPECT_EQ(countChannels(d3.get()), 1);

}

class MockAngleListener : public Listener<Angle<>> {
 public:
  MOCK_METHOD1(onNewValue, void(const ValueDispatcher<Angle<>> &dispatcher));
};

TEST(DispatcherUtilsTest, Replay2) {
  ReplayDispatcher replay;
  replay.setCurrentTime(TimeStamp::now());
  MockAngleListener listener, twaListener;

  // First subscription
  replay.get<AWA>()->dispatcher()->subscribe(&listener);

  // We expect the callback when publishing a value.
  EXPECT_CALL(listener, onNewValue(testing::_));
  replay.publishValue(AWA, "test source", Angle<>::degrees(42));

  // Second subscription for the listener. Since a listener only
  // listen to 1 channel at a time, the first subscription is canceled.
  replay.get<TWA>()->dispatcher()->subscribe(&listener);

  // So the following publish will not trigger a callback
  replay.advanceTime(Duration<>::seconds(1));
  replay.publishValue(AWA, "test source", Angle<>::degrees(43));
  
  // However, the 2nd subscription still works.
  EXPECT_CALL(listener, onNewValue(testing::_));
  replay.publishValue(TWA, "test source", Angle<>::degrees(44));
}
