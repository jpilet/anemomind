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

  KDE1 estimator(1.0, Array<KDE1::Vec>::args(KDE1::Vec{c}));

  for (int i = 0; i < 30; i++) {
    double a = sin(i*123.324342);
    double b = sin(i*857.374522);
    EXPECT_EQ(std::abs(a - c) < std::abs(b - c),
              estimator.density(KDE1::Vec{a}) > estimator.density(KDE1::Vec{b}));
  }
}

TEST(DensityEstimatorTest, NaiveTest1dWithTwoPoints) {
  // A single training sample for the distribution.
  double c = 3.9;
  double d = 34.0;

  KDE1 estimator(1.0, Array<KDE1::Vec>::args(KDE1::Vec{c},
                                    KDE1::Vec{d}));

  for (int i = 0; i < 30; i++) {
    double a = sin(i*123.324342);
    double b = sin(i*857.374522);
    double amin = std::min(std::abs(a - c), std::abs(a - d));
    double bmin = std::min(std::abs(b - c), std::abs(b - d));
    EXPECT_EQ(amin < bmin,
              estimator.density(KDE1::Vec{a}) > estimator.density(KDE1::Vec{b}));
  }

  double middle = 0.5*(c + d);
  double middleDensity = estimator.density(KDE1::Vec{middle});

  for (int i = 0; i < 30; i++) {
    double x = double(i)/29;
    double y = c + x*(d - c);
    EXPECT_LE(middleDensity, estimator.density(KDE1::Vec{y}));
  }
}

TEST(DensityEstimatorTest, BandWidth) {
  KDE1::Vec a = KDE1::Vec{3.4};
  KDE1::Vec b = KDE1::Vec{9.89};

  KDE1 deWide(30, Array<KDE1::Vec>::args(a));
  KDE1 deNarrow(0.89, Array<KDE1::Vec>::args(a));
  EXPECT_LE(deNarrow.density(b)/deNarrow.density(a),
            deWide.density(b)/deWide.density(a));
}

TEST(DensityEstimatorTest, TwoDims) {
  KDE2::Vec a = KDE2::Vec{0, 0};
  KDE2::Vec b = KDE2::Vec{-1, -1};
  KDE2::Vec c = KDE2::Vec{-0.5, 20};

  KDE2 estimator(1, Array<KDE2::Vec>::args(a));
  EXPECT_LE(estimator.density(c), estimator.density(b));
  EXPECT_LE(estimator.density(b), estimator.density(a));
}


