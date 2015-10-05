/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/irls.h>
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
  // Fit a line to a signal subject to sparsity constraints.
  // We allow for exactly two discontinuities in the fitted signal.


  // Build the noisy signal
  Arrayd noisySignal(30);
  Arrayd time(30);
  Arrayd gt(30);
  for (int i = 0; i < 30; i++) {
    gt[i] = (i < 10 || i > 20? 1 : 0);
    noisySignal[i] = gt[i] + 0.1*sin(34*i*i);
    time[i] = i;
  }

  // Build the A matrix and the B vector.
  // The upper part of A is an identity matrix,
  // and the upper part of B is the noisy signal.
  // This corresponding to fitting our denoised signal to the data.
  // The lower part of A and B are 29 regularization equations, among which
  // 27 should evaluate to 0. Our solver will optimize the choice of the remaining
  // two nonzero regularity equations.
  int rows = 30 + 29;
  Eigen::VectorXd B = Eigen::VectorXd::Zero(rows);
  typedef Eigen::Triplet<double> Triplet;
  std::vector<Triplet> triplets;
  triplets.reserve(30 + 2*29);
  for (int i = 0; i < 30; i++) {
    triplets.push_back(Triplet(i, i, 1.0));
    B(i) = noisySignal[i];
  }
  Array<Spani> cst(29);
  for (int i = 0; i < 29; i++) {
    int row = 30 + i;
    triplets.push_back(Triplet(row, i, 1.0));
    triplets.push_back(Triplet(row, i+1, -1.0));
    cst[i] = Spani(row, row+1);
  }
  Eigen::SparseMatrix<double> A(rows, 30);
  A.setFromTriplets(triplets.begin(), triplets.end());
  SparsityConstrained::Settings settings;
  settings.iters = 8;

  // Since we have a total of 29 constraints, and we allow
  // for two discontinuities (that will be passive constraints),
  // there remains 27 active constraints.
  SparsityConstrained::ConstraintGroup group{cst, 27};
  auto X = SparsityConstrained::solve(A, B,
      Array<SparsityConstrained::ConstraintGroup>{group}, settings);
  for (int i = 0; i < 30; i++) {
    EXPECT_NEAR(X(i), gt[i], 0.02);
  }

  // This will show the noisy signal and the denoised signal.
  constexpr bool visualize = false;
  if (visualize) {
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.plot_xy(time, noisySignal);
    plot.plot_xy(time, Arrayd(X.size(), X.data()));
    plot.show();
  }
}
