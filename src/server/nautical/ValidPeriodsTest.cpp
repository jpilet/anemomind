#include <server/nautical/ValidPeriods.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {

TimeStamp t(int x) {
  static TimeStamp base = TimeStamp::now();

  return base + Duration<>::seconds(x);
}

}  // namespace 

TEST(ValidPeriods, MergeTest) {
  StatusTimedVector a{
    TimedValue<StatusChange>(t(0), StatusChange::toValid),
    TimedValue<StatusChange>(t(10), StatusChange::toInvalid),
    TimedValue<StatusChange>(t(15), StatusChange::toValid),
    TimedValue<StatusChange>(t(20), StatusChange::toInvalid)
  };

  StatusTimedVector b{
    TimedValue<StatusChange>(t(1), StatusChange::toValid),
    TimedValue<StatusChange>(t(12), StatusChange::toInvalid),
    TimedValue<StatusChange>(t(24), StatusChange::toValid),
    TimedValue<StatusChange>(t(27), StatusChange::toInvalid)
  };

  StatusTimedVector aUb = statusVectorUnion(a, b);

  StatusTimedVector ref{
    TimedValue<StatusChange>(t(0), StatusChange::toValid),
    TimedValue<StatusChange>(t(12), StatusChange::toInvalid),
    TimedValue<StatusChange>(t(15), StatusChange::toValid),
    TimedValue<StatusChange>(t(20), StatusChange::toInvalid),
    TimedValue<StatusChange>(t(24), StatusChange::toValid),
    TimedValue<StatusChange>(t(27), StatusChange::toInvalid)
  };

  CHECK_EQ(ref.size(), aUb.size());
  for (int i = 0; i < ref.size(); ++i) {
    CHECK_EQ(ref[i].time, aUb[i].time);
    CHECK_EQ(ref[i].value, aUb[i].value);
  }
}

TEST(ValidPeriods, BasicIterate) {
  StatusTimedVector a{
    TimedValue<StatusChange>(t(0), StatusChange::toValid),
    TimedValue<StatusChange>(t(12), StatusChange::toInvalid),
    TimedValue<StatusChange>(t(15), StatusChange::toValid),
    TimedValue<StatusChange>(t(20), StatusChange::toInvalid),
    TimedValue<StatusChange>(t(24), StatusChange::toValid),
    TimedValue<StatusChange>(t(27), StatusChange::toInvalid)
  };

  int i = 0;
  ValidPeriods vp(&a);
  for (ValidPeriods::iterator it = vp.begin(); it != vp.end(); ++it) {
    Period p = *it;
    EXPECT_EQ(a[i * 2].time, p.begin);
    EXPECT_EQ(a[i * 2 + 1].time, p.end);
    ++i;
  }

  i = 0;
  for (Period p : ValidPeriods(&a)) {
    EXPECT_EQ(a[i * 2].time, p.begin);
    EXPECT_EQ(a[i * 2 + 1].time, p.end);
    ++i;
  }
}

