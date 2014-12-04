/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/Flow.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(FlowTest, Constant) {
  Flow f = Flow::constant(Velocity<double>::knots(3.0), Velocity<double>::knots(2.0));
  EXPECT_NEAR(f(Flow::ProjectedPosition(), Duration<double>())[0].knots(), 3.0, 1.0e-6);
  EXPECT_NEAR(f(Flow::ProjectedPosition(), Duration<double>())[1].knots(), 2.0, 1.0e-6);
}

TEST(FlowTest, Add) {
  Flow f = Flow::constant(Velocity<double>::knots(3.0), Velocity<double>::knots(2.0)) + Flow();
  EXPECT_NEAR(f(Flow::ProjectedPosition(), Duration<double>())[0].knots(), 3.0, 1.0e-6);
  EXPECT_NEAR(f(Flow::ProjectedPosition(), Duration<double>())[1].knots(), 2.0, 1.0e-6);
}

TEST(FlowTest, Add0) {
  Flow f = Flow() + Flow();
  EXPECT_NEAR(f(Flow::ProjectedPosition(), Duration<double>())[0].knots(), 0.0, 1.0e-6);
  EXPECT_NEAR(f(Flow::ProjectedPosition(), Duration<double>())[1].knots(), 0.0, 1.0e-6);
}

