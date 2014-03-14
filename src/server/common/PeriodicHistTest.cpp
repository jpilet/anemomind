/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include "PeriodicHist.h"

using namespace sail;

TEST(PeriodicHistTest, PHTestIndexer) {
  PeriodicHistIndexer indexer(2);
  EXPECT_EQ(indexer.toBin(Angle<double>::radians(0)), 0);
  EXPECT_EQ(indexer.toBin(Angle<double>::radians(M_PI)), 1);
  EXPECT_EQ(indexer.toBin(Angle<double>::radians(1.49*M_PI)), 1);
  EXPECT_EQ(indexer.toBin(Angle<double>::radians(1.51*M_PI)), 0);
  EXPECT_EQ(indexer.toBin(Angle<double>::radians(4*M_PI)), 0);
}

TEST(PeriodicHistTest, PHTestIndexerLeftMiddleRight) {
  PeriodicHistIndexer indexer(2);
  EXPECT_NEAR(indexer.binLeft(0).radians(), -0.5*M_PI, 1.0e-9);
  EXPECT_NEAR(indexer.binMiddle(0).radians(), 0, 1.0e-9);
  EXPECT_NEAR(indexer.binRight(0).radians(), 0.5*M_PI, 1.0e-9);
}

TEST(PeriodicHistTest, PHTestAdd) {
  PeriodicHist hist(2);
  hist.add(Angle<double>::radians(0), 1.0);
  hist.add(Angle<double>::radians(0.9*M_PI), 2.0);
  hist.add(Angle<double>::radians(0.3*M_PI), 2.5);
  EXPECT_NEAR(hist.value(Angle<double>::radians(0)), 3.5, 1.0e-9);
  EXPECT_NEAR(hist.value(Angle<double>::radians(M_PI)), 2.0, 1.0e-9);
}

TEST(PeriodicHistTest, PHTestPlotData) {
  PeriodicHist hist(4);
  for (int i = 0; i < 4; i++) {
    hist.add(hist.indexer().binMiddle(i), 1.0);
  }
  MDArray2d data = hist.makePolarPlotData();

  double k = 1.0/sqrt(2.0);
  for (int i = 0; i < data.rows(); i++) {
    int local = i % 3;
    if (local == 1 || local == 2) {
      EXPECT_NEAR(std::abs(data(i, 0)), k, 1.0e-9);
      EXPECT_NEAR(std::abs(data(i, 1)), k, 1.0e-9);
    } else {
      EXPECT_NEAR(data(i, 0), 0, 1.0e-9);
      EXPECT_NEAR(data(i, 1), 0, 1.0e-9);
    }
  }
}

TEST(PeriodicHistTest, PHTestDiv) {

  const int bc = 3;
  PeriodicHistIndexer indexer(bc);
  double adata[bc] = {1.0, 2.0, 2.0};
  double bdata[bc] = {0.5, 2.0, 4.0};
  double expectedCdata[bc] = {2.0, 1.0, 0.5};

  PeriodicHist A(indexer, Arrayd(bc, adata));
  PeriodicHist B(indexer, Arrayd(bc, bdata));
  PeriodicHist C = A/B;
  Arrayd cdata = C.data();
  for (int i = 0; i < bc; i++) {
    EXPECT_NEAR(cdata[i], expectedCdata[i], 1.0e-9);
  }
}

