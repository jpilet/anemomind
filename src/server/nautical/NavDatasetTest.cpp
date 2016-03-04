/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/NavDataset.h>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/DispatcherUtils.h>

using namespace sail;

namespace {
  auto offset = TimeStamp::UTC(2016, 02, 25, 12, 07, 0);

  std::shared_ptr<Dispatcher> makeTestDispatcher() {

    sail::TimedSampleCollection<Velocity<double> >::TimedVector samples;
    for (int i = 0; i < 60; i++) {
      samples.push_back(TimedValue<Velocity<double> >(offset + Duration<double>::seconds(i), Velocity<double>::knots(0.3*i)));
    }
    EXPECT_EQ(samples.size(), 60);

    auto d = std::make_shared<Dispatcher>();
    d->insertValues<Velocity<double> >(AWS, "MastAnemometer", samples);

    return d;
  }
}

TEST(NavDatasetTest, ConstructionAndSlicing) {
  auto testDispatcher = makeTestDispatcher();
  NavDataset ds(testDispatcher);
  EXPECT_FALSE(ds.hasLowerBound());
  EXPECT_FALSE(ds.hasUpperBound());
  auto bounded = ds.fitBounds();
  EXPECT_TRUE(bounded.hasLowerBound());
  EXPECT_TRUE(bounded.hasUpperBound());
  EXPECT_NEAR((bounded.lowerBound() - offset).seconds(), 0.0, 1.0e-6);
  EXPECT_NEAR((bounded.upperBound() - offset).seconds(), 59.0, 1.0e-6);
  auto samples = bounded.samples<AWS>();

  EXPECT_EQ(samples, ds.samples<AWS>());

  EXPECT_LT(0, samples.size());

  auto firstHalf = bounded.sliceTo(offset + Duration<double>::seconds(29.5));
  EXPECT_EQ(firstHalf.samples<AWS>().size(), 30);

  EXPECT_NEAR(firstHalf.samples<AWS>().last().value.knots(), 0.3*29, 1.0e-6);

  {
    auto a = firstHalf.samples<AWS>()[0];
    auto b = firstHalf.samples<AWS>().first();
    EXPECT_EQ(a.time, b.time);
    EXPECT_EQ(a.value, b.value);
  }

  {
    auto bs = bounded.samples<AWS>();
    int i = 0;
    for (auto x: bs) {
      EXPECT_NEAR((x.time - offset).seconds(), i, 1.0e-6);
      i++;
    }
    Optional<TimedValue<Velocity<double> > > x = bs.nearest(offset + Duration<double>::seconds(13.0));
    EXPECT_TRUE(x.defined());
    auto value = x.get();
    EXPECT_NEAR((value.time - offset).seconds(), 13.0, 1.0e-6);
    EXPECT_NEAR(value.value.knots(), 0.3*13, 1.0e-6);
  }{
    Optional<TimedValue<Velocity<double> > > x = firstHalf.samples<AWS>().nearest(offset + Duration<double>::seconds(13.0));
    EXPECT_TRUE(x.defined());
    auto value = x.get();
    EXPECT_NEAR((value.time - offset).seconds(), 13.0, 1.0e-6);
    EXPECT_NEAR(value.value.knots(), 0.3*13, 1.0e-6);
  }

  EXPECT_TRUE(bounded.samples<GPS_POS>().empty());

  bounded.outputSummary(&(std::cout));
}

TEST(NavDatasetTest, EmptyTest) {
  NavDataset coll;
  EXPECT_TRUE(coll.samples<AWA>().empty());
}
