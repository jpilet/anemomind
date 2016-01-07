
#include <device/anemobox/TimedSampleCollection.h>

#include <array>
#include <gtest/gtest.h>
#include <random>

using namespace sail;
using namespace std;

namespace {

void randomArray(deque<TimedValue<int>> *array, int offset = 0) {
  static auto rng = default_random_engine(7);

  TimeStamp base = TimeStamp::now();

  std::array<int, 100> count;
  for (int i = 0; i < count.size(); ++i) {
    count[i] = i + offset;
  }
  shuffle(count.begin(), count.end(), rng);

  for (int i = 0; i < count.size(); ++i) {
    TimeStamp t = base + Duration<>::seconds(count[i]);
    array->push_back(TimedValue<int>(t, count[i]));
  }
}

}  // namespace

TEST(TimedSampleCollection, BatchInsert) {
  deque<TimedValue<int>> random1;
  randomArray(&random1);

  TimedSampleCollection<int> samples(random1);

  for (int i = 0; i < random1.size(); ++i) {
    EXPECT_EQ(i, samples.samples()[i].value);
  }

  EXPECT_EQ(samples.size(), random1.size());

  deque<TimedValue<int>> random2;
  randomArray(&random2, random1.size());
  samples.insert(random2);

  EXPECT_EQ(samples.size(), random1.size() + random2.size());

  for (int i = 0; i < samples.samples().size(); ++i) {
    EXPECT_EQ(i, samples.samples()[i].value);
  }
}

TEST(TimedSampleCollection, Nearest) {
  TimedSampleCollection<int> samples;
  TimeStamp base = TimeStamp::now();
  samples.append(TimedValue<int>(base, 1));
  samples.append(TimedValue<int>(base + Duration<>::seconds(1), 2));
  samples.append(TimedValue<int>(base + Duration<>::seconds(2), 3));

  EXPECT_FALSE(samples.nearest(base - Duration<>::seconds(1)).defined());
  EXPECT_EQ(1, samples.nearest(base + Duration<>::seconds(.1))());
  EXPECT_EQ(2, samples.nearest(base + Duration<>::seconds(.9))());
  EXPECT_EQ(2, samples.nearest(base + Duration<>::seconds(1.1))());
  EXPECT_EQ(3, samples.nearest(base + Duration<>::seconds(1.6))());
}

TEST(TimedSampleCollection, MaxBufferLength) {
  deque<TimedValue<int>> random1;
  randomArray(&random1);

  TimedSampleCollection<int> samples(20);
  samples.insert(random1);

  for (int i = 0; i < 20; ++i) { EXPECT_EQ(80 + i, samples.samples()[i].value); }

  samples.append(TimedValue<int>(TimeStamp::now() + Duration<>::seconds(100), 100));

  for (int i = 0; i < 20; ++i) { EXPECT_EQ(81 + i, samples.samples()[i].value); }
}

