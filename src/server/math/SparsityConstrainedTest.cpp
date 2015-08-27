/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SparsityConstrained.h>
#include <gtest/gtest.h>
#include <server/plot/extra.h>

using namespace sail;

TEST(SparsityConstrained, DistributeWeights) {
  Arrayd R0{1.0, 1.0e-9};
  Arrayd W0 = SparsityConstrained::distributeWeights(R0, 9.9);
  EXPECT_EQ(W0.size(), 2);
  EXPECT_NEAR(W0[0], 0.0, 1.0e-5);
  EXPECT_NEAR(W0[1], 2.0*9.9, 1.0e-5);

  Arrayd R1{1.0, 2.0, 3.0};
  Arrayd W1 = SparsityConstrained::distributeWeights(R1, 3.4);
  EXPECT_EQ(W1.size(), 3);
  EXPECT_NEAR(W1[0] + W1[1] + W1[2], 3*3.4, 1.0e-5);

  double smallGap = 0.1;
  EXPECT_LT(W1[2], W1[1] + smallGap);
  EXPECT_LT(W1[1], W1[0] + smallGap);
}

TEST(SparsityConstrained, SignalFit) {
  Arrayd noisySignal(300);
  Arrayd time(300);
  for (int i = 0; i < 300; i++) {
    noisySignal[i] = (i < 100 || i > 200? 1 : 0) + 0.1*sin(34*i*i);
    time[i] = i;
  }

  int rows = 300 + 299;
  Eigen::VectorXd B = Eigen::VectorXd::Zero(rows);
  typedef Eigen::Triplet<double> Triplet;
  std::vector<Triplet> triplets;
  triplets.reserve(300 + 2*299);
  for (int i = 0; i < 300; i++) {
    triplets.push_back(Triplet(i, i, 1.0));
    B(i) = noisySignal[i];
  }
  Array<Spani> cst(299);
  for (int i = 0; i < 299; i++) {
    int row = 300 + i;
    triplets.push_back(Triplet(row, i, 1.0));
    triplets.push_back(Triplet(row, i+1, -1.0));
    cst[i] = Spani(row, row+1);
  }
  Eigen::SparseMatrix<double> A(rows, 300);
  A.setFromTriplets(triplets.begin(), triplets.end());
  SparsityConstrained::Settings settings;
  auto X = SparsityConstrained::solve(A, B, cst, 297, settings);


  if (true) {
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.plot_xy(time, noisySignal);
    plot.plot_xy(time, Arrayd(X.size(), X.data()));
    plot.show();
  }
}
