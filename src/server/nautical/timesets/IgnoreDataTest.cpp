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

TEST(IgnoreDataTest, TestIt) {
  TimeSetInterval ivl;

  typedef TimeSetTypes TST;

  Array<TimeSetInterval> intervals{
    {TST::ignoreButVisualize, Span<TimeStamp>(t(0), t(4))},
    {TST::merge, Span<TimeStamp>(t(5), t(7.9))},
    {TST::ignoreCompletely, Span<TimeStamp>(t(8), t(12))},
  };

  auto ignore = ignoreDispatchData(intervals, {
      TST::ignoreButVisualize,
      TST::ignoreCompletely
  });

  auto src = std::make_shared<
      TypedDispatchDataReal<Velocity<double>>>(
      WAT_SPEED, "k", nullptr, -1);

  {
    TimedSampleCollection<Velocity<double>>::TimedVector vec;
    for (double x = 0.0; x < 16; x += 0.1) {
      vec.push_back(TimedValue<Velocity<double>>(t(x), x*1.0_kn));
    }
    src->dispatcher()->insert(vec);
  }

  auto dst = ignore(std::static_pointer_cast<DispatchData>(src));
  EXPECT_TRUE(bool(dst));
  std::cout << "Code: " << descriptionForCode(dst->dataCode()) << "\n";
  auto typedDst = std::dynamic_pointer_cast<
      TypedDispatchData<Velocity<double>>>(dst);
  EXPECT_TRUE(bool(typedDst));

  auto samples = typedDst->dispatcher()->values();

  EXPECT_LT(10, samples.size());

}





