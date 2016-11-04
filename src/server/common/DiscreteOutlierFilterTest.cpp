/*
 * DiscreteOutlierFilterTest.cpp
 *
 *  Created on: 4 Nov 2016
 *      Author: jonas
 */

#include <server/common/DiscreteOutlierFilter.h>
#include <gtest/gtest.h>

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
