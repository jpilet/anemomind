/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/SparseFilter.h>

using namespace sail;

TEST(SparseFilterTest, TestFilter) {
  int count = 300;
  Sampling sampling = Sampling::identity(300);

  Array<Observation<1> > observations(count);
  for (int i = 0; i < 300; i++) {
    int index = i*17 % 300;
    double y = (index < 200? 0 : 1) + 0.1*sin(i) + 0.05*sin(34*i + 23443);

    // Some outliers
    if (i < 9) {
      y = 30*sin(234432*i + 234234);
    }

    observations[i] = Observation<1>{Sampling::Weights::atIndex(index), {y}};
  }

  int inlierCount = 290;
  SparseFilter::Settings settings;
  auto results = SparseFilter::filter(sampling, observations, inlierCount, 1, settings);

}


