/*
 * BoatStateReconstructorTest.cpp
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#include <server/nautical/calib/BoatStateReconstructor.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {
  TimeStamp offsetTime = TimeStamp::UTC(2016, 9, 22, 15, 17, 0);

  TimeStamp t(double s) {
    return offsetTime + Duration<double>::seconds(s);
  }

  TimeStampToIndexMapper mapper{
    offsetTime, 2.0_s, 4};
}

TEST(BoatStateReconstructor, TimeMapper) {
  EXPECT_EQ(mapper.offset, offsetTime);
  EXPECT_EQ(mapper.period, 2.0_s);
  EXPECT_EQ(mapper.sampleCount, 4);
  EXPECT_EQ(mapper.map(offsetTime - 1.0_s), -1);
  EXPECT_EQ(mapper.map(offsetTime - 9.0_s), -1);
  EXPECT_EQ(mapper.map(offsetTime + 1.0_s), 0);
  EXPECT_EQ(mapper.map(offsetTime + 3.0_s), 1);
  EXPECT_EQ(mapper.map(offsetTime + 9.0_s), -1);

  struct Call {
    int index;
    Spani span;
  };
  std::vector<Call> calls;

  Array<TimedValue<Velocity<double> > > values{
    {t(-11.3), 3.4_kn},
    {t(0.1), 3.4_kn},
    {t(1.0), 4.4_kn},
    {t(1.5), 4.5_kn},
    {t(5), 9.0_kn},
    {t(11), 9.4_kn}
  };
  foreachSpan<Velocity<double> >(mapper, values,
      [&](int index, Spani span) {
    calls.push_back(Call{index, span});
  });
  EXPECT_EQ(calls.size(), 2);
  auto c0 = calls[0];
  auto c1 = calls[1];
  EXPECT_EQ(c0.index, 0);
  EXPECT_EQ(c0.span.minv(), 1);
  EXPECT_EQ(c0.span.maxv(), 4);
  EXPECT_EQ(c1.index, 2);
  EXPECT_EQ(c1.span.minv(), 4);
  EXPECT_EQ(c1.span.maxv(), 5);
}

TEST(BoatStateReconstructor, ValueAccumulator) {

}
/*TEST(BoatStateReconstructor, BasicTest) {
  Array<BoatState<double> > initialStates(1);
  BoatStateReconstructor<double, ServerBoatStateSettings>
    reconstructor(offsetTime, 1.0_s, initialStates);

}*/
