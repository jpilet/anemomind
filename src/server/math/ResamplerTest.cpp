/*
 * ResamplerTest.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <iostream>
#include <server/common/ArrayIO.h>
#include <server/math/Resampler.h>
#include <gtest/gtest.h>

using namespace sail;
using namespace sail::Resampler;

TEST(ResamplerTest, TestEps) {
  Arrayi samples{30, 40};
  int period = 1;
  auto eps = listEndpoints(samples, period);
  EXPECT_EQ(eps.size(), 4);
  int times[4] = {29, 31, 39, 41};
  for (int i = 0; i < 4; i++) {
    EXPECT_EQ(times[i], eps[i].pos);
    EXPECT_EQ(eps[i].rising, i % 2 == 0);
  }
  auto newSamples = makeNewSamplesFromEndpoints(eps, period);
  EXPECT_EQ(newSamples, (Arrayi{30, 40}));
}


TEST(ResamplerTest, TestEpsDense) {
  Arrayi samples{30, 31, 34, 35, 37, 40};
  int period = 2;
  auto eps = listEndpoints(samples, period);
  EXPECT_EQ(eps.size(), 12);
  int times[12] = {28, 29, 32, 32, 33, 33, 35, 36, 37, 38, 39, 42};
  for (int i = 0; i < 12; i++) {
    EXPECT_EQ(times[i], eps[i].pos);
  }
  auto newSamples = makeNewSamplesFromEndpoints(eps, period);
  EXPECT_EQ(newSamples, (Arrayi{30, 32, 34, 36, 38, 40}));
}
