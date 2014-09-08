/*
 *  Created on: 2014-09-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/DensityEstimator.h>
#include <cmath>

using namespace sail;

namespace {
}

TEST(DensityEstimatorTest, NaiveTest1d) {
  // A single training sample for the distribution.
  double c = 3.9;

  KernelDensityEstimator<1> estimator(1.0, Array<KernelDensityEstimator<1>::Vec>::args(KernelDensityEstimator<1>::Vec{c}));

  for (int i = 0; i < 30; i++) {
    double a = sin(i*123.324342);
    double b = sin(i*857.374522);
    EXPECT_EQ(std::abs(a - c) < std::abs(b - c),
              estimator.density(KernelDensityEstimator<1>::Vec{a}) > estimator.density(KernelDensityEstimator<1>::Vec{b}));
  }
}

TEST(DensityEstimatorTest, NaiveTest1dWithTwoPoints) {
  // A single training sample for the distribution.
  double c = 3.9;
  double d = 34.0;

  KernelDensityEstimator<1> estimator(1.0, Array<KernelDensityEstimator<1>::Vec>::args(KernelDensityEstimator<1>::Vec{c},
                                    KernelDensityEstimator<1>::Vec{d}));

  for (int i = 0; i < 30; i++) {
    double a = sin(i*123.324342);
    double b = sin(i*857.374522);
    double amin = std::min(std::abs(a - c), std::abs(a - d));
    double bmin = std::min(std::abs(b - c), std::abs(b - d));
    EXPECT_EQ(amin < bmin,
              estimator.density(KernelDensityEstimator<1>::Vec{a}) > estimator.density(KernelDensityEstimator<1>::Vec{b}));
  }

  double middle = 0.5*(c + d);
  double middleDensity = estimator.density(KernelDensityEstimator<1>::Vec{middle});

  for (int i = 0; i < 30; i++) {
    double x = double(i)/29;
    double y = c + x*(d - c);
    EXPECT_LE(middleDensity, estimator.density(KernelDensityEstimator<1>::Vec{y}));
  }
}

TEST(DensityEstimatorTest, BandWidth) {
  KernelDensityEstimator<1>::Vec a = KernelDensityEstimator<1>::Vec{3.4};
  KernelDensityEstimator<1>::Vec b = KernelDensityEstimator<1>::Vec{9.89};

  KernelDensityEstimator<1> deWide(30, Array<KernelDensityEstimator<1>::Vec>::args(a));
  KernelDensityEstimator<1> deNarrow(0.89, Array<KernelDensityEstimator<1>::Vec>::args(a));
  EXPECT_LE(deNarrow.density(b)/deNarrow.density(a),
            deWide.density(b)/deWide.density(a));
}

TEST(DensityEstimatorTest, TwoDims) {
  KernelDensityEstimator<2>::Vec a = KernelDensityEstimator<2>::Vec{0, 0};
  KernelDensityEstimator<2>::Vec b = KernelDensityEstimator<2>::Vec{-1, -1};
  KernelDensityEstimator<2>::Vec c = KernelDensityEstimator<2>::Vec{-0.5, 20};

  KernelDensityEstimator<2> estimator(1, Array<KernelDensityEstimator<2>::Vec>::args(a));
  EXPECT_LE(estimator.density(c), estimator.density(b));
  EXPECT_LE(estimator.density(b), estimator.density(a));
}


