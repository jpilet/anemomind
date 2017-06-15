/*
 * SeqOutlierFilterTest.cpp
 *
 *  Created on: 9 Jun 2017
 *      Author: jonas
 */
#include <server/math/SeqOutlierFilter.h>
#include <gtest/gtest.h>
#include <server/common/math.h>
#include <Eigen/Dense>

class ShortStraightLineModel {
public:
  double cost() const {
    return counter < 3? 0.0
        : (values[0] - 2*values[1] + values[2]).squaredNorm();
  }

  ShortStraightLineModel insert(const Eigen::Vector2d value) const {
    ShortStraightLineModel next = *this;
    next.counter++;

    // Shift left
    next.values[0] = values[1];
    next.values[1] = values[2];
    next.values[2] = value;

    return next;
  }
private:
  int counter = 0;
  Eigen::Vector2d values[3];
};

std::pair<int >

TEST(SeqOutlierFilterTest, ShortLineTest) {
  using namespace sail;
  using namespace SeqOutlierFilter;

  Settings settings;
  settings.maxLocalModelCost = 1.0;
  settings.maxLocalModelCount = 20;
  State<Eigen::Vector2d, ShortStraightLineModel> state(
      settings, ShortStraightLineModel());

  int n = 300;
  int outlierIndex = 67;
  double outlierValue = -3000;
  double k = 3.4;
  double m = -23;

  for (int i = 0; i < n; i++) {
    //if (i == outlierIndex)
  }

}
