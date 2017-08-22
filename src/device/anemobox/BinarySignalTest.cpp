
#include <device/anemobox/BinarySignal.h>
#include <server/common/logging.h>

#include <gtest/gtest.h>

using namespace sail;

namespace {

TimeStamp t(int x) {
  static TimeStamp base = TimeStamp::now();

  return base + Duration<>::seconds(x);
}

}  // namespace 

TEST(BinarySignal, MergeTest) {
  BinarySignal a{
    makeTimedValue(t(0), BinaryEdge::ToOn),
    makeTimedValue(t(10), BinaryEdge::ToOff),
    makeTimedValue(t(15), BinaryEdge::ToOn),
    makeTimedValue(t(20), BinaryEdge::ToOff)
  };

  BinarySignal b{
    makeTimedValue(t(1), BinaryEdge::ToOn),
    makeTimedValue(t(12), BinaryEdge::ToOff),
    makeTimedValue(t(24), BinaryEdge::ToOn),
    makeTimedValue(t(27), BinaryEdge::ToOff)
  };

  BinarySignal aUb = binarySignalUnion(a, b);

  BinarySignal ref{
    makeTimedValue(t(0), BinaryEdge::ToOn),
    makeTimedValue(t(12), BinaryEdge::ToOff),
    makeTimedValue(t(15), BinaryEdge::ToOn),
    makeTimedValue(t(20), BinaryEdge::ToOff),
    makeTimedValue(t(24), BinaryEdge::ToOn),
    makeTimedValue(t(27), BinaryEdge::ToOff)
  };

  CHECK_EQ(ref.size(), aUb.size());
  for (int i = 0; i < ref.size(); ++i) {
    CHECK_EQ(ref[i].time, aUb[i].time);
    CHECK_EQ(int(ref[i].value), int(aUb[i].value));
  }
}

TEST(BinarySignal, BasicIterate) {
  BinarySignal a{
    makeTimedValue(t(0), BinaryEdge::ToOn),
    makeTimedValue(t(12), BinaryEdge::ToOff),
    makeTimedValue(t(15), BinaryEdge::ToOn),
    makeTimedValue(t(20), BinaryEdge::ToOff),
    makeTimedValue(t(24), BinaryEdge::ToOn),
    makeTimedValue(t(27), BinaryEdge::ToOff)
  };

  int i = 0;
  OnPeriods vp(&a);
  for (OnPeriods::iterator it = vp.begin(); it != vp.end(); ++it) {
    Period p = *it;
    EXPECT_EQ(a[i * 2].time, p.begin);
    EXPECT_EQ(a[i * 2 + 1].time, p.end);
    ++i;
  }

  i = 0;
  for (Period p : OnPeriods(&a)) {
    EXPECT_EQ(a[i * 2].time, p.begin);
    EXPECT_EQ(a[i * 2 + 1].time, p.end);
    ++i;
  }
}

