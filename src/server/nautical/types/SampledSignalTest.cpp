/*
 * DiscreteTimeSignal.cpp
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#include <server/nautical/types/SampledSignal.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {

  class VelocitySignal : public SampledSignal<Velocity<double> > {
  public:
    TimedValue<Velocity<double> > operator[] (int i) const override {
      return TimedValue<Velocity<double> >(
          TimeStamp::UTC(2016, 5, 13, 13, 55, i),
          Velocity<double>::knots(i));
    }

    size_t size() const override {
      return 4;
    }
  private:
  };

  std::shared_ptr<SampledSignal<Velocity<double> > > makeTheSignal() {
    return std::shared_ptr<SampledSignal<Velocity<double> > >(new VelocitySignal());
  }
}

TEST(DTSignalTest, TestIt) {
  auto signal = makeTheSignal();
  EXPECT_EQ(signal->size(), 4);
  EXPECT_TRUE(signal->chronologicallyOrdered());
  {
    auto x = (*signal)[2];
    EXPECT_EQ(x.time, TimeStamp::UTC(2016, 5, 13, 13, 55, 2));
    EXPECT_NEAR(x.value.knots(), 2.0, 1.0e-6);
  }{
    auto x0 = signal->evaluate(TimeStamp::UTC(2016, 5, 13, 13, 55, 2.3));
    EXPECT_TRUE(x0.defined());
    auto x = x0.get();
    EXPECT_EQ(x.time, TimeStamp::UTC(2016, 5, 13, 13, 55, 2));
    EXPECT_NEAR(x.value.knots(), 2.0, 1.0e-6);
  }{
    auto x0 = signal->evaluate(TimeStamp::UTC(2016, 5, 13, 13, 55, 2.7));
    EXPECT_TRUE(x0.defined());
    auto x = x0.get();
    EXPECT_EQ(x.time, TimeStamp::UTC(2016, 5, 13, 13, 55, 3));
    EXPECT_NEAR(x.value.knots(), 3.0, 1.0e-6);
  }
}
