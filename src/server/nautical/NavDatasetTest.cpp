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
  const int numSamples = 60;

  std::shared_ptr<Dispatcher> makeTestDispatcher() {

    sail::TimedSampleCollection<Velocity<double> >::TimedVector samples;
    for (int i = 0; i < numSamples; i++) {
      samples.push_back(TimedValue<Velocity<double> >(offset + Duration<double>::seconds(i), Velocity<double>::knots(0.3*i)));
    }
    EXPECT_EQ(samples.size(), 60);

    auto d = std::make_shared<Dispatcher>();
    d->insertValues<Velocity<double> >(AWS, "NMEA0183: test", samples);

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

  bounded.outputSummary(&(std::cout));
}

TEST(NavDatasetTest, EmptyTest) {
  NavDataset coll;
  EXPECT_TRUE(coll.samples<AWA>().empty());
}

namespace {
  auto s = Duration<double>::seconds(1.0);
  auto kn = Velocity<double>::knots(1.0);
}

TEST(NavDatasetTest, TestAddChannels) {
  const DataCode Code = AWS;
  typedef TypeForCode<Code>::type T;
  typedef TimedSampleCollection<T>::TimedVector TimedVector;


  TimedVector A{
    {offset + 0.3*s,    3.0*kn},
    {offset + 0.6*s,    4.0*kn},
    {offset + 1233.0*s, 13.0*kn}
  };

  TimedVector A2{
    {offset + 0.4*s,    17.0*kn}
  };

  NavDataset ds;
  NavDataset ds2 = ds.addChannel<T>(Code, "NMEA0183: A", A);

  EXPECT_FALSE(bool(ds.dispatcher()));
  EXPECT_TRUE(bool(ds2.dispatcher()));
  EXPECT_EQ(ds2.samples<Code>().size(), 3);
  EXPECT_NEAR(ds2.samples<Code>()[1].value.knots(), 4.0, 1.0e-6);

  NavDataset ds3 = ds2.replaceChannel<T>(Code, "NMEA0183: A", A2);
  EXPECT_TRUE(bool(ds3.dispatcher()));
  EXPECT_EQ(ds3.samples<Code>().size(), 1);
}

TEST(NavDatasetTest, TestReplaceChannel) {
  TimedSampleCollection<Velocity<>> aws;
  aws.append(TimedValue<Velocity<>>(offset + 0.0 * s, 13.0 * kn));
  aws.append(TimedValue<Velocity<>>(offset + 1.0 * s, 13.1 * kn));
  aws.append(TimedValue<Velocity<>>(offset + 2.0 * s, 13.2 * kn));
  aws.append(TimedValue<Velocity<>>(offset + 3.0 * s, 13.3 * kn));

  std::shared_ptr<Dispatcher> dispatcher = makeTestDispatcher();
  NavDataset navs(dispatcher);

  NavDataset modified = navs.replaceChannel<Velocity<>>(
      AWS, "NMEA0183: replaced", aws.samples());

  EXPECT_EQ("NMEA0183: test", navs.dispatcher()->dispatchData(AWS)->source());
  EXPECT_EQ("NMEA0183: replaced", modified.dispatcher()->dispatchData(AWS)->source());
  EXPECT_EQ(13.3, modified.dispatcher()->val<AWS>().knots());
}

TEST(NavDatasetTest, StripChannel) {
  NavDataset navs(makeTestDispatcher());

  EXPECT_EQ("NMEA0183: test", navs.dispatcher()->dispatchData(AWS)->source());
  EXPECT_LT(0, navs.dispatcher()->values<AWS>().size());

  NavDataset stripped(navs.stripChannel(AWS));
  EXPECT_EQ(0, stripped.dispatcher()->values<AWS>().size());
}

TEST(NavDatasetTest, CreateMergedChannel) {
  // 60 samples AWS
  std::shared_ptr<Dispatcher> dispatcher = makeTestDispatcher();
  NavDataset navs(dispatcher);

  dispatcher->setSourcePriority("NMEA0183: test", 2);
  dispatcher->setSourcePriority("NMEA0183: test2", 0);

  TimedSampleCollection<Velocity<>>::TimedVector aws{
    // This sample should be ignored, because it overlaps with the other
    // channel and the prio is lower
    {offset + 5.0 *s, 13.0*kn},
    {offset + 1000.0 *s, 13.0*kn},
    {offset + 1001.0 *s, 13.1*kn},
  };

  NavDataset twoChans =
    navs.addChannel<Velocity<>>(AWS, "NMEA0183: test2", aws);

  NavDataset merged = twoChans.createMergedChannels(std::set<DataCode>{AWS});

  auto samples = merged.samples<AWS>();
  EXPECT_EQ(numSamples + 2, samples.size());

  // All the values for the first channel
  for (int i = 0; i < numSamples; ++i) {
    EXPECT_EQ(0.3 * i, samples[i].value.knots());
  }

  // The two values from the second channel
  EXPECT_EQ(13.0, samples[numSamples + 0].value.knots());
  EXPECT_EQ(13.1, samples[numSamples + 1].value.knots());

}

TEST(NavDatasetTest, FailIfReadingUnmerged) {
  NavDataset navs(makeTestDispatcher());

  TimedSampleCollection<Velocity<>>::TimedVector aws{
    {offset + 5.0 *s, 13.0*kn},
    {offset + 1001.0 *s, 13.1*kn},
  };

  NavDataset twoChans =
    navs.addChannel<Velocity<>>(AWS, "NMEA0183: test2", aws);

  // AWS contains two channel, we merge AWA (but not AWS)
  // so, there are still two channels in unmerged.
  NavDataset unmerged = twoChans.createMergedChannels(std::set<DataCode>{AWA});

  // Trying to read without a valid channel selection is an error
  EXPECT_ANY_THROW(unmerged.samples<AWS>());
}

