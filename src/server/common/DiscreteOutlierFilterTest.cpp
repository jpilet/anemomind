/*
 * DiscreteOutlierFilterTest.cpp
 *
 *  Created on: 4 Nov 2016
 *      Author: jonas
 */

#include <server/common/DiscreteOutlierFilter.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(DiscreteOutlierFilterTest, BasicTest) {

  Array<double> values{7, 8, 130, 3, 4};
  DiscreteOutlierFilter::Settings settings;
  settings.threshold = 20;

  auto mask = DiscreteOutlierFilter::computeOutlierMask<double*, double>(
      values.begin(), values.end(),
      [](double a, double b) {return std::abs(a - b);},
      settings);

  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(values[i], i != 2);
  }
}
