/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/DataFit.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/plot/extra.h>

using namespace sail;
using namespace DataFit;

TEST(DataFitTest, Indexer) {
  CoordIndexer::Factory rows;
  CoordIndexer positions = rows.make(12, 3);
  EXPECT_EQ(rows.count(), 36);
  EXPECT_EQ(positions.from(), 0);
  EXPECT_EQ(positions.to(), 36);
  EXPECT_EQ(positions.numel(), 36);
  EXPECT_EQ(positions.elementSpan(), Spani(0, 36));
  EXPECT_EQ(positions.coordinateSpan(), Spani(0, 12));

  CoordIndexer slack = rows.make(12, 1);
  EXPECT_EQ(rows.count(), 48);
  EXPECT_EQ(slack.elementSpan(), Spani(36, 48));
  EXPECT_EQ(slack.coordinateSpan(), Spani(0, 12));

}

bool operator== (Triplet a, Triplet b) {
  return a.row() == b.row() && a.col() == b.col() && a.value() == b.value();
}


// Simulate offset error in some positions.
TEST(DataFit, QuadraticFit) {
  int n = 30;
  int dataCount = n - 1;
  int k = 4;
  Array<Observation<1> > observations(dataCount);
  Arrayd X(dataCount), Y(dataCount), Ygt(dataCount);
  for (int i = 0; i < dataCount; i++) {
    double offset = (i < k? 30 : 0);
    X[i] = i;
    Ygt[i] = 0.3*i;
    Y[i] = Ygt[i] + 0.1*sin(exp(3.0*i)) + offset;
    observations[i] = Observation<1>{Sampling::Weights{i, 1.0, 0.0}, {Y[i]}};
  }
  irls::Settings settings;
  settings.iters = 20;
  auto results = DataFit::quadraticFitWithInliers(n, observations, 0.2, 2, 1.0, settings);

  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(i*0.3, results.samples(i, 0), 0.4);
  }

  bool visualize = false;
  if (visualize) {
    GnuplotExtra plot;
    plot.plot_xy(X, Y);
    plot.set_style("lines");
    plot.plot_xy(X, Ygt);
    plot.plot_xy(X, results.samples.getStorage().sliceBut(1));
    plot.show();
  }
}
