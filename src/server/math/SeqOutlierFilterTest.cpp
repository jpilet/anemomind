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
  } else if (index < 300) {
    return {0, trueValue};
  }
  return {3, trueValue};
}

double trueFunction(int index) {
  return index < 300? 3.4*index - 23 : 56 - 0.1*index;
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

  // Problem description:
  // We have a curve that should be broken up into two sections.
  // The first section is for the first 300 samples and is a straight
  // line with slope 3.4 and offset -23. The second section is
  // a straight line for the remaining samples from index 300,
  // with slope -0.1 and offset 56.
  //
  // We have injected a few outliers in the data, and the objective
  // of this code is to
  //  (i) Assign segment indices to all the samples
  // (ii) Determine which segment indices are likely corresponding
  //      to inlier data.

  // Part 1: Assign segment indices to every sample
  IndexGrouper grouper;
  int n = 400;
  for (int i = 0; i < n; i++) {
    auto indexAndValue = getDataWithOutliers(i, trueFunction(i));
    auto expectedSegmentIndex = indexAndValue.first;
    auto value = indexAndValue.second;
    int segmentIndex = state.filter(Eigen::Vector2d(i, value));
    EXPECT_EQ(expectedSegmentIndex, segmentIndex);
    grouper.insert(segmentIndex);
  }

  // Part 2: Parse the sequence of segment indices
  // to extract only inlier segments. We expect two sections,
  // as described in the problem description.
  auto inliers = computeInlierSegments(grouper.get());

  EXPECT_EQ(inliers.size(), 2);
  EXPECT_TRUE(inliers.count(0) == 1);
  EXPECT_TRUE(inliers.count(3) == 1);
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

