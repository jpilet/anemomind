/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/SparseFilter.h>
#include <server/plot/extra.h>

using namespace sail;

TEST(SparseFilterTest, TestFilter) {
  int count = 30;
  Sampling sampling = Sampling::identity(30);

  MDArray2d input(30, 2);

  Array<Observation<1> > observations(count);
  for (int i = 0; i < 30; i++) {
    int index = (7 + i*17) % 30;
    double y = (index < 20? 0 : 1) + 0.1*sin(i) + 0.05*sin(34*i + 23443);

    // Some outliers
    if (i < 4) {
      y = 8*sin(234.432*i + 2342.34);
    }

    input(i, 0) = index;
    input(i, 1) = y;

    observations[i] = Observation<1>{Sampling::Weights::atIndex(index), {y}};
  }

  int inlierCount = 24;
  SparseFilter::Settings settings;
  auto results = SparseFilter::filter(sampling, observations, inlierCount, 1, settings);

  for (int i = 0; i < 30; i++) {
    EXPECT_NEAR(results(i, 0), i < 20? 0 : 1, 0.02);
  }

  constexpr bool visualize = false;
  if (visualize) {
    MDArray2d filtered(30, 2);
    for (int i = 0; i < 30; i++) {
      filtered(i, 0) = i;
      filtered(i, 1) = results(i, 0);
    }
    GnuplotExtra plot;
    plot.set_style("points");
    plot.plot(input);
    plot.set_style("lines");
    plot.plot(filtered);
    plot.show();
  }
}


