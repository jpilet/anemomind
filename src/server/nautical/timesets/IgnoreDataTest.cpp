/*
 * IgnoreDataTest.cpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#include <server/nautical/timesets/IgnoreData.h>
#include <gtest/gtest.h>

using namespace sail;

TimeStamp t(double s) {
  return TimeStamp::UTC(2017, 9, 7, 16, 15, 0) + s*1.0_s;
}

bool hasValueNear(
    const TimedSampleCollection<Velocity<double>>::TimedVector& v,
    double x) {
  auto target = t(x);
  for (auto y: v) {
    if (fabs((y.time - target).seconds()) < 0.2) {
      return true;
    }
  }
  return false;
}


typedef TimeSetTypes TST;

Array<TimeSetInterval> getIntervals() {
  return {
    {TST::ignoreButVisualize, Span<TimeStamp>(t(1), t(4))},
    {TST::merge, Span<TimeStamp>(t(5), t(7.9))},
    {TST::ignoreCompletely, Span<TimeStamp>(t(8), t(12))},
  };
}

std::set<std::string> getToIgnore() {
  return {
    TST::ignoreButVisualize,
    TST::ignoreCompletely
  };
}

std::shared_ptr<DispatchData> makeTestDispatchData() {
  auto src = std::make_shared<
      TypedDispatchDataReal<Velocity<double>>>(
      WAT_SPEED, "k", nullptr,
      std::numeric_limits<int>::max());

  {
    TimedSampleCollection<Velocity<double>>::TimedVector vec;
    for (double x = 0.0; x < 16; x += 0.1) {
      vec.push_back(TimedValue<Velocity<double>>(t(x), x*1.0_kn));
    }
    src->dispatcher()->insert(vec);
  }
  return src;
}

void checkDispatchData(const std::shared_ptr<DispatchData>& dst) {
  EXPECT_TRUE(bool(dst));
  std::cout << "Code: " << descriptionForCode(dst->dataCode()) << "\n";
  auto typedDst = std::dynamic_pointer_cast<
      TypedDispatchData<Velocity<double>>>(dst);
  EXPECT_TRUE(bool(typedDst));

  auto samples = typedDst->dispatcher()->values().samples();

  EXPECT_LT(10, samples.size());
  auto first = samples.front();
  EXPECT_NEAR((first.time - t(0)).seconds(), 0.0, 0.2);
  EXPECT_NEAR(first.value.knots(), 0.0, 0.2);

  auto last = samples.back();
  EXPECT_NEAR((last.time - t(16)).seconds(), 0.0, 0.2);
  EXPECT_NEAR(last.value.knots(), 16.0, 0.2);

  EXPECT_TRUE(hasValueNear(samples, 0.5));
  EXPECT_TRUE(hasValueNear(samples, 6.0));
  EXPECT_FALSE(hasValueNear(samples, 2.0));
  EXPECT_FALSE(hasValueNear(samples, 9.0));
}

TEST(IgnoreDataTest, TestIt) {

  auto ignore = ignoreDispatchData(getIntervals(), getToIgnore());
  auto src = makeTestDispatchData();

  auto dst = ignore(std::static_pointer_cast<DispatchData>(src));

  checkDispatchData(dst);

}





