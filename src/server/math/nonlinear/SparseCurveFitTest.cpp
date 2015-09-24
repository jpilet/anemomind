/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/SparseCurveFit.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>

using namespace sail;
using namespace SparseCurveFit;


bool operator== (Triplet a, Triplet b) {
  return a.row() == b.row() && a.col() == b.col() && a.value() == b.value();
}

TEST(SparseCurveFitTest, RegTest) {
  std::vector<Triplet> dst;
  auto spans = makeReg(2, 10, 1,
      2, 2, &dst);
  EXPECT_EQ(spans.size(), 2);
  EXPECT_EQ(spans[0], Spani(10, 12));
  EXPECT_EQ(spans[1], Spani(12, 14));

  EXPECT_TRUE(dst[0] == Triplet(10, 1, 1.0));
  EXPECT_TRUE(dst[1] == Triplet(11, 2, 1.0));
  EXPECT_TRUE(dst[2] == Triplet(10, 3, -2.0));
  EXPECT_TRUE(dst[3] == Triplet(11, 4, -2.0));
  EXPECT_TRUE(dst[4] == Triplet(10, 5, 1.0));
  EXPECT_TRUE(dst[5] == Triplet(11, 6, 1.0));

  EXPECT_TRUE(dst[6] == Triplet(12, 3, 1.0));
  EXPECT_TRUE(dst[7] == Triplet(13, 4, 1.0));
  EXPECT_TRUE(dst[8] == Triplet(12, 5, -2.0));
  EXPECT_TRUE(dst[9] == Triplet(13, 6, -2.0));
  EXPECT_TRUE(dst[10] == Triplet(12, 7, 1.0));
  EXPECT_TRUE(dst[11] == Triplet(13, 8, 1.0));

  EXPECT_EQ(dst.size(), 12);
}

TEST(SparseCurveFitTest, DataTest) {
  Array<Observation<2> > obs{Observation<2>{Sampling::Weights{0, 0.3, 0.7}, {119.3, 119.4}}};

  Eigen::VectorXd rhs = Eigen::VectorXd::Zero(30);
  std::vector<Triplet> triplets;
  auto spans = makeDataFitness<2>(3, 1,
      obs, 4,
      &triplets, &rhs);

  EXPECT_EQ(spans.size(), 1);

  EXPECT_EQ(triplets.size(), 8);
  EXPECT_EQ(rhs(3), 119.3);
  EXPECT_EQ(rhs(4), 119.4);
  for (int i = 0; i < rhs.size(); i++) {
    EXPECT_TRUE(rhs(i) == 0 || i == 3 || i == 4);
  }

  Spani rowRange, colRange;
  for (auto t: triplets) {
    rowRange.extend(t.row());
    colRange.extend(t.col());
  }
  EXPECT_EQ(rowRange.minv(), 3);
  EXPECT_EQ(rowRange.maxv() + 1, 3 + 2 + 2);
  EXPECT_EQ(rowRange.maxv() + 1, spans.last().maxv());
  EXPECT_EQ(spans[0], Spani(5, 7));
  EXPECT_EQ(colRange.minv(), 1);
  EXPECT_EQ(colRange.maxv() + 1, 1 + 2*4 + 2);

  MDArray2d data(rowRange.maxv()+1, colRange.maxv()+1);
  data.setAll(0.0);
  for (auto t: triplets) {
    data(t.row(), t.col()) = t.value();
  }
  EXPECT_EQ(data(4, 2), 0.3);
  EXPECT_EQ(data(3, 3), 0.7);
  EXPECT_EQ(data(3 + 2 + 1, 1 + 2*4 + 1), 1.0);
}

TEST(SparseCurveFit, AssembeResultsTest) {
  Eigen::VectorXd solution(4 + 4);

  // Sample 1
  solution(0) = 1;
  solution(1) = 2;

  // Sample 2
  solution(2) = 3;
  solution(3) = 4;

  // Slack 1
  solution(4) = 0.1;
  solution(5) = 0.12;

  // Slack 2 (inlier)
  solution(6) = 0.00001;
  solution(7) = 0.0012;

  auto results = assembleResults(2, 2, 1,
      solution);
  EXPECT_EQ(results.samples(0, 0), 1.0);
  EXPECT_EQ(results.samples(0, 1), 2.0);
  EXPECT_EQ(results.samples(1, 0), 3.0);
  EXPECT_EQ(results.samples(1, 1), 4.0);

  EXPECT_EQ(results.inliers, (Arrayb{false, true}));
}

TEST(SparseCurveFit, NoisyStep) {
  int sampleCount = 30;

  Array<Observation<1> > observations(sampleCount-1);

  auto gtSignal = [&](double index) {
    return (2*index < sampleCount? -1 : 2);
  };

  for (int i = 0; i < sampleCount-1; i++) {
    double noise = 0.1*sin(30*exp(i) + i*i);
    double y = gtSignal(i) + noise;
    observations[i] = Observation<1>{Sampling::Weights{i, 1.0, 0.0}, {y}};
  }

  Settings settings;
  settings.regOrder = 1;
  settings.inlierRate = 1.0;
  auto results = fit(sampleCount, 1, observations, settings);

  for (auto x: results.inliers) {
    EXPECT_TRUE(x);
  }
  EXPECT_EQ(results.inliers.size(), observations.size());
  EXPECT_EQ(results.samples.cols(), 1);
  for (int i = 0; i < results.samples.rows(); i++) {
    EXPECT_NEAR(gtSignal(i), results.samples(i, 0), 0.02);
  }
}
