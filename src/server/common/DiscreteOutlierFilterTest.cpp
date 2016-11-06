/*
 * DiscreteOutlierFilterTest.cpp
 *
 *  Created on: 4 Nov 2016
 *      Author: jonas
 */

#include <server/common/DiscreteOutlierFilter.h>
#include <gtest/gtest.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(DiscreteOutlierFilterTest, BasicTest1) {

  Array<double> values{7, 8, 130, 3, 4};
  DiscreteOutlierFilter::Settings settings;
  settings.threshold = 20;

  auto mask = DiscreteOutlierFilter::computeOutlierMask<double*, double>(
      values.begin(), values.end(),
      [](double a, double b) {return std::abs(a - b);},
      settings);

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(mask[i], i != 2);
  }
}

TEST(DiscreteOutlierFilterTest, BasicTest2) {

  Array<double> values{7, 130, 0, 139, 4};
  DiscreteOutlierFilter::Settings settings;
  settings.threshold = 20;

  auto mask = DiscreteOutlierFilter::computeOutlierMask<double*, double>(
      values.begin(), values.end(),
      [](double a, double b) {return std::abs(a - b);},
      settings);

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(mask[i], (i != 1 && i != 3));
  }
}

TEST(DiscreteOutlierFilterTest, BasicTest3) {

  Array<double> values;
  DiscreteOutlierFilter::Settings settings;
  settings.threshold = 20;

  EXPECT_TRUE((DiscreteOutlierFilter::computeOutlierMask<double*, double>(
      values.begin(), values.end(),
      [](double a, double b) {return std::abs(a - b);},
      settings).empty()));
}

TEST(DiscreteOutlierFilterTest, BasicTest4) {

  Array<double> values{1, 2, 3, 2, 2, 2, 2, 4, 5, 100, 10001, 300, 3, 4, 5};
  DiscreteOutlierFilter::Settings settings;
  settings.threshold = 20;

  auto mask = DiscreteOutlierFilter::computeOutlierMask<double*, double>(
      values.begin(), values.end(),
      [](double a, double b) {return std::abs(a - b);},
      settings);
  EXPECT_EQ(mask, (Array<bool>{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1
  }));
}

TEST(DiscreteOutlierFilterTest, BasicTest5) {

  Array<double> values{1};
  DiscreteOutlierFilter::Settings settings;
  settings.threshold = 20;

  auto mask = DiscreteOutlierFilter::computeOutlierMask<double*, double>(
      values.begin(), values.end(),
      [](double a, double b) {return std::abs(a - b);},
      settings);
  EXPECT_EQ(mask, (Array<bool>{
    1
  }));
}

TEST(DiscreteOutlierFilterTest, FindSpan) {
  auto offset = TimeStamp::UTC(2016, 11, 6, 10, 59, 0);
  DiscreteOutlierFilter::TimeSpanIndexer indexer(Array<Span<TimeStamp>>{
    Span<TimeStamp>(offset, offset + 3.0_s),
    Span<TimeStamp>(offset + 5.0_s, offset + 7.0_s),
    Span<TimeStamp>(offset + 7.1_s, offset + 8.0_s)
  });
  EXPECT_EQ(-1, indexer.lookUp(offset - 1.0_s));
  EXPECT_EQ(0, indexer.lookUp(offset + 1.0_s));
  EXPECT_EQ(0, indexer.lookUp(offset + 4.0_s));
  EXPECT_EQ(2, indexer.lookUp(offset + 7.5_s));
  EXPECT_EQ(-1, indexer.lookUp(offset + 11.5_s));
}


namespace {
  auto offset = TimeStamp::UTC(2016, 11, 6, 10, 59, 0);

  TimedValue<double> tv(double t, double v) {
    return TimedValue<double>(offset + t*1.0_s, v);
  }

  double costDouble(TimedValue<double> a, TimedValue<double> b) {
    return std::abs((a.value - b.value)/(
        1.0 + std::abs((a.time - b.time).seconds())));
  }
}

TEST(DiscreteOutlierFilterTest, TimedTest0) {
  auto data = Array<TimedValue<double>>{
    tv(0.0, 1.0), tv(1.0, 2.0), tv(2.0, 3.0),
        tv(3.0, 1134.0), tv(4.0, 1135.0)
  };

  auto outliers = DiscreteOutlierFilter::identifyOutliers<double>(
      data, &costDouble,
      Array<Duration<double>>{
        1.0_s, 2.0_s, 4.0_s, 8.0_s,
        16.0_s, 32.0_s},
      9.0);
  EXPECT_EQ(outliers, (Array<bool>{
    1, 1, 1, 0, 0
  }));
}

TEST(DiscreteOutlierFilterTest, TimedTest1) {
  auto data = Array<TimedValue<double>>{
    tv(0.0, 1.0), tv(1.0, 2.0), tv(2.0, 3.0),
        tv(3.0, 1134.0), tv(4.0, 1135.0),
        tv(4.1, 7.0), tv(8.0, 7.0)
  };

  auto outliers = DiscreteOutlierFilter::identifyOutliers<double>(
      data, &costDouble,
      Array<Duration<double>>{
        1.0_s, 2.0_s, 4.0_s, 8.0_s,
        16.0_s, 32.0_s},
      9.0);
  EXPECT_EQ(outliers, (Array<bool>{
    1, 1, 1, 0, 0, 1, 1
  }));
}

TEST(DiscreteOutlierFilterTest, TimedTest2) {
  auto data = Array<TimedValue<double>>{
    tv(0.0, 1.0)
  };

  auto outliers = DiscreteOutlierFilter::identifyOutliers<double>(
      data, &costDouble,
      Array<Duration<double>>{
        1.0_s, 2.0_s, 4.0_s, 8.0_s,
        16.0_s, 32.0_s},
      9.0);
  EXPECT_EQ(outliers, (Array<bool>{
    1
  }));
}

TEST(DiscreteOutlierFilterTest, TimedTest3) {
  auto data = Array<TimedValue<double>>{};

  auto outliers = DiscreteOutlierFilter::identifyOutliers<double>(
      data, &costDouble,
      Array<Duration<double>>{
        1.0_s, 2.0_s, 4.0_s, 8.0_s,
        16.0_s, 32.0_s},
      9.0);
  EXPECT_EQ(outliers, (Array<bool>{}));
}

TEST(DiscreteOutlierFilterTest, TimedTest4) {
  auto data = Array<TimedValue<double>>{
    tv(0.0, 0.0),
    tv(1.0, 0.0),
    tv(2.0, 0.0),
    tv(3.0, 0.0),
    tv(4.0, 0.0),
    tv(5.0, 0.0),
    tv(6.0, 0.0),
    tv(7.0, 0.0),
    tv(8.0, 10000.0),
    tv(9.0, 3000.0),
    tv(10.0, 900000.0),
    tv(11.0, 99.0),
    tv(12.0, 0.0),
    tv(13.0, 0.0),
    tv(14.0, 0.0),
    tv(15.0, 0.0),
    tv(16.0, 0.0),
  };

  auto outliers = DiscreteOutlierFilter::identifyOutliers<double>(
      data, &costDouble,
      Array<Duration<double>>{
        1.0_s, 2.0_s, 4.0_s, 8.0_s,
        16.0_s, 32.0_s},
      9.0);
  EXPECT_EQ(outliers, (Array<bool>{
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1
  }));
}


