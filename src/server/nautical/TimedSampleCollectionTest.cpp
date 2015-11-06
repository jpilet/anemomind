
#include <server/nautical/TimedSampleCollection.h>

#include <array>
#include <gtest/gtest.h>
#include <random>

using namespace sail;
using namespace std;

TEST(TimedSampleCollection, Insert) {
  TimedSampleCollection<int> samples;

  std::array<int, 100> count;
  for (int i = 0; i < count.size(); ++i) {
    count[i] = i;
  }
  shuffle(count.begin(), count.end(), default_random_engine(7));

  TimeStamp base = TimeStamp::now();

  for (int i = 0; i < count.size(); ++i) {
    TimeStamp t = base + Duration<>::seconds(count[i]);
    samples.insert(TimedValue<int>(t, count[i]));
  }

  for (int i = 0; i < count.size(); ++i) {
    EXPECT_EQ(i, samples.samples()[i].value);
  }
}

TEST(TimedSampleCollection, Nearest) {
  TimedSampleCollection<int> samples;
  TimeStamp base = TimeStamp::now();
  samples.insert(TimedValue<int>(base, 1));
  samples.insert(TimedValue<int>(base + Duration<>::seconds(1), 2));
  samples.insert(TimedValue<int>(base + Duration<>::seconds(2), 3));

  EXPECT_FALSE(samples.nearest(base - Duration<>::seconds(1)).defined());
  EXPECT_EQ(1, samples.nearest(base + Duration<>::seconds(.1))());
  EXPECT_EQ(2, samples.nearest(base + Duration<>::seconds(.9))());
  EXPECT_EQ(2, samples.nearest(base + Duration<>::seconds(1.1))());
  EXPECT_EQ(3, samples.nearest(base + Duration<>::seconds(1.6))());
}
