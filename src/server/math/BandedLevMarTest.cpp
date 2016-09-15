/*
 * BandedLevMarTest.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include <server/math/BandedLevMar.h>
#include <gtest/gtest.h>
#include <random>
#include <server/common/LineKM.h>

using namespace sail;
using namespace sail::BandedLevMar;

namespace {
  struct DataCost{
    double data;

    int inputCount() const {return 1;}
    int outputCount() const {return 1;}

    template <typename T>
    bool evaluate(const T *X, T *y) const {
      y[0] = X[0] - data;
      return true;
    }
  };

}

TEST(BandedLevmarTest, BasicLineFit) {
  std::default_random_engine rng(0);

  std::uniform_real_distribution<double> distrib(0.0, 4.0);

  int n = 30;

  LineKM gtLine(0, n, 3.4, 9.5);

  Problem<double> problem;
  for (int i = 0; i < n; i++) {
    problem.addCostFunction(Spani(i, i+1),
        new DataCost{gtLine(i) + distrib(rng)});
  }
}



