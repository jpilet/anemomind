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
  std::cout << "mask: " << mask << std::endl;
}
