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
#include <server/common/LineKM.h>

using namespace sail;

// This is a model of a short straight line.
// Drawing a line through the two last observations,
// it measures how well does the next observation fits to that line.
class ShortStraightLineModel {
public:
  double previewCost(const Eigen::Vector2d& next) const {
    if (counter < 2) {
      return 0.0;
    }

    LineKM line(values[0](0), values[1](0),
        values[0](1), values[1](1));
    return sqr(line(next(0)) - next(1));
  }

  void insert(const Eigen::Vector2d value) {
    values[0] = values[1];
    values[1] = value;
    counter++;
  }
private:
  int counter = 0;
  Eigen::Vector2d values[2];
};

std::pair<int, double> getDataWithOutliers(int index, double trueValue) {
  if (7 <= index && index < 20) {
    return {1, trueValue + 1000};
  } else if ((index == 30) || (index == 40)) {
    return {2, 90};
  }
  return {0, trueValue};
}

double trueFunction(int index) {
  return 3.4*index - 23;
}

TEST(SeqOutlierFilterTest, ModelTest) {
  ShortStraightLineModel model;

  EXPECT_EQ(0.0, model.previewCost(Eigen::Vector2d(0, 0)));
  EXPECT_EQ(0.0, model.previewCost(Eigen::Vector2d(1, 0.5)));
  model.insert(Eigen::Vector2d(0, 0));
  model.insert(Eigen::Vector2d(1.0, 0.5));
  model.previewCost(Eigen::Vector2d(3.0, 1.5));
}

using namespace sail;
using namespace SeqOutlierFilter;

TEST(SeqOutlierFilterTest, ShortLineTest) {

  Settings settings;
  settings.maxLocalModelCost = 1.0;
  settings.maxLocalModelCount = 20;
  State<Eigen::Vector2d, ShortStraightLineModel> state(
      settings, ShortStraightLineModel());

  int n = 300;
  for (int i = 0; i < n; i++) {
    auto indexAndValue = getDataWithOutliers(i, trueFunction(i));
    auto expectedSegmentIndex = indexAndValue.first;
    auto value = indexAndValue.second;
    int segmentIndex = state.filter(Eigen::Vector2d(i, value));
    EXPECT_EQ(expectedSegmentIndex, segmentIndex);
  }
}

TEST(SeqOutlierFilterTest, IndexGrouperTest) {
  IndexGrouper grouper;
  grouper.insert(0);
  grouper.insert(0);
  grouper.insert(1);
  grouper.insert(1);
  grouper.insert(1);
  grouper.insert(1);
  grouper.insert(4);
  auto groups = grouper.get();
  EXPECT_EQ(3, groups.size());

  int inds[3] = {0, 1, 4};
  int froms[3] = {0, 2, 6};
  int tos[3] = {2, 6, 7};
  for (int i = 0; i < 3; i++) {
    auto g = groups[i];
    EXPECT_EQ(g.index, inds[i]);
    EXPECT_EQ(g.span.minv(), froms[i]);
    EXPECT_EQ(g.span.maxv(), tos[i]);
  }
}
