/*
 * LazyReplayDispatchDataTest.cpp
 *
 *  Created on: 1 Jun 2017
 *      Author: jonas
 */

#include <device/anemobox/LazyReplayDispatchData.h>
#include <gtest/gtest.h>

using namespace sail;

typedef Velocity<double> T;

class SampleSource : public Clock {
public:
  SampleSource(std::function<double(double)> g) :
    _g(g), _counter(0) {}

  TimeStamp currentTime() {
    return TimeStamp::UTC(2017, 6, 1, 11, 3, 0)
      + double(_counter)*1.0_s;
  }

  TimedValue<Velocity<double>> generate() {
    _counter++;
    return TimedValue<T>(
        currentTime(), _g(_counter)*1.0_mps);
  }

  int counter() const {return _counter;}

  SampleSource reset() const {return SampleSource(_g);}
private:
  std::function<double(double)> _g;
  int _counter;
};

bool matches(
    const std::shared_ptr<LazyReplayDispatchData<T>>& data,
    const SampleSource& src) {
  const auto& samples = data->dispatcher()->values().samples();
  if (samples.size() != src.counter()) {
    LOG(INFO) << "Sample count in DispatchData: " << samples.size();
    LOG(INFO) << "Expected sample count: " << src.counter();
    LOG(INFO) << "Sample count mismatch";
    return false;
  }

  auto r = src.reset();
  for (auto x: samples) {
    auto y = r.generate();
    if (!(x.time == y.time && x.value == y.value)) {
      LOG(INFO) << "Sample mismatch";
      return false;
    }
  }
  return true;
}


auto dc = DataCode::AWA;
auto src = "MySensor";

TEST(LazyReplayDispatchData, VariousTests) {
  int n = 5;
  auto fn = [](double x) {return 0.3*x + 0.1;};
  const SampleSource original(fn);

  auto protoSrc = original;
  auto prototype = std::make_shared<TypedDispatchDataReal<T>>(
      dc, src, &protoSrc, 20);

  { // Populate the prototype
    for (int i = 0; i < n; i++) {
      prototype->setValue(protoSrc.generate().value);
    }
  }

  // Try different cases.

  { // Duplication
    auto otherSrc = original;
    auto dst = std::make_shared<LazyReplayDispatchData<T>>(
        &otherSrc, 2,
        std::static_pointer_cast<TypedDispatchData<T>>(prototype));

    for (int i = 0; i < n; i++) {
      dst->setValue(otherSrc.generate().value);
    }
    EXPECT_TRUE(dst->finalize());
    EXPECT_TRUE(matches(dst, otherSrc));
    EXPECT_EQ(dst->dispatcher(), prototype->dispatcher());
  }{ // Only a subset
    auto otherSrc = original;
    auto dst = std::make_shared<LazyReplayDispatchData<T>>(
        &otherSrc, 2,
        std::static_pointer_cast<TypedDispatchData<T>>(prototype));

    for (int i = 0; i < n-1; i++) {
      dst->setValue(otherSrc.generate().value);
    }
    EXPECT_TRUE(dst->finalize());
    EXPECT_TRUE(matches(dst, otherSrc));
    EXPECT_NE(dst->dispatcher(), prototype->dispatcher());
  }{ // Partially the same
    auto fn2 = [fn](double x) {return x < 4? fn(x) : 9.0;};
    SampleSource otherSrc(fn2);
    auto dst = std::make_shared<LazyReplayDispatchData<T>>(
        &otherSrc, 2,
        std::static_pointer_cast<TypedDispatchData<T>>(prototype));

    for (int i = 0; i < n; i++) {
      dst->setValue(otherSrc.generate().value);
    }
    EXPECT_TRUE(dst->finalize());
    EXPECT_TRUE(matches(dst, otherSrc));
    EXPECT_NE(dst->dispatcher(), prototype->dispatcher());
  }{ // Longer
    auto otherSrc = original;
    auto dst = std::make_shared<LazyReplayDispatchData<T>>(
        &otherSrc, 2,
        std::static_pointer_cast<TypedDispatchData<T>>(prototype));

    for (int i = 0; i < n+1; i++) {
      dst->setValue(otherSrc.generate().value);
    }
    EXPECT_TRUE(dst->finalize());
    EXPECT_TRUE(matches(dst, otherSrc));
    EXPECT_NE(dst->dispatcher(), prototype->dispatcher());
  }{ // Empty
    auto otherSrc = original;
    auto dst = std::make_shared<LazyReplayDispatchData<T>>(
        &otherSrc, 2,
        std::static_pointer_cast<TypedDispatchData<T>>(prototype));
    EXPECT_FALSE(dst->finalize());
    EXPECT_TRUE(matches(dst, otherSrc));
    EXPECT_NE(dst->dispatcher(), prototype->dispatcher());
  }
}
