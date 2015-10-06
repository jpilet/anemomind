/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/irls.h>
#include <gtest/gtest.h>
#include <server/plot/extra.h>
#include <server/common/string.h>

typedef Eigen::Triplet<double> Triplet;





using namespace sail;

TEST(IrlsTest, DistributeWeights) {
  Arrayd R0{1.0, 1.0e-9};
  Arrayd W0 = irls::distributeWeights(R0, 9.9);
  EXPECT_EQ(W0.size(), 2);
  EXPECT_NEAR(W0[0], 0.0, 1.0e-5);
  EXPECT_NEAR(W0[1], 2.0*9.9, 1.0e-5);

  Arrayd R1{1.0, 2.0, 3.0};
  Arrayd W1 = irls::distributeWeights(R1, 3.4);
  EXPECT_EQ(W1.size(), 3);
  EXPECT_NEAR(W1[0] + W1[1] + W1[2], 3*3.4, 1.0e-5);

  double smallGap = 0.1;
  EXPECT_LT(W1[2], W1[1] + smallGap);
  EXPECT_LT(W1[1], W1[0] + smallGap);
}

TEST(IrlsTest, SignalFit) {
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
  irls::Settings settings;
  settings.iters = 8;

  // Since we have a total of 29 constraints, and we allow
  // for two discontinuities (that will be passive constraints),
  // there remains 27 active constraints.
  Array<std::shared_ptr<irls::WeightingStrategy> > strategies{
    std::shared_ptr<irls::WeightingStrategy>(new irls::ConstraintGroup(cst, 27))
  };

  auto X = irls::solve(A, B,
      strategies, settings);
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


// Minimize X + X subject to X >= 3 and X >= 2
TEST(IrlsTest, InequalityConstraint) {
  using namespace irls;

  Array<Triplet> triplets{
    Triplet(0, 0, 1),
    Triplet(1, 0, 1),
    };
  Eigen::VectorXd B(2);
  Eigen::SparseMatrix<double> A(2, 1);
  A.setFromTriplets(triplets.begin(), triplets.end());
  B(0) = 3;
  B(1) = 2;
  WeightingStrategies strategies{
    InequalityConstraint::make(Arrayi{0, 1}, 1.0)
  };

  Settings settings;
  settings.iters = 200;
  auto results = solve(A, B, strategies, settings);
  double x = results(0);
  EXPECT_NEAR(x, 3.0, 1.0e-8);
}

// Minimize X - k subject to |X| < 1
TEST(IrlsTest, BoundedNormConstraint) {
  using namespace irls;
  Array<Triplet> triplets{
    Triplet(0, 0, 1.0),
    Triplet(1, 0, 1.0)
  };
  Eigen::SparseMatrix<double> A(2, 1);
  A.setFromTriplets(triplets.begin(), triplets.end());

  double K[3] = {3.0, 0.7, -2.0};
  double Xgt[3] = {1.0, 0.7, -1.0};

  WeightingStrategies strategies{
    BoundedNormConstraint::make(Array<Spani>{Spani(1, 2)}, 1.0)
  };

  for (int i = 0; i < 3; i++) {
    Eigen::VectorXd B(2);
    B(0) = K[i];
    B(1) = 0.0;
    Settings settings;
    settings.iters = 200;

    auto results = solve(A, B, strategies, settings);
    double x = results(0);
    EXPECT_NEAR(x, Xgt[i], 1.0e-4);
  }
}

void constantNormConstraint(const double *target2, const double *gt2) {
  using namespace irls;

  Array<Triplet> triplets{
    Triplet(0, 0, 1.0),
    Triplet(1, 1, 1.0),
    Triplet(2, 0, 1.0),
    Triplet(3, 1, 1.0)
  };
  Eigen::SparseMatrix<double> A(4, 2);
  A.setFromTriplets(triplets.begin(), triplets.end());

  Eigen::VectorXd B = Eigen::VectorXd::Zero(4);
  B(0) = target2[0];
  B(1) = target2[1];

  WeightingStrategies strategies{
    ConstantNormConstraint::make(Array<Spani>{Spani(2, 4)}, 1.0)
  };

  Settings settings;
  settings.iters = 200;
  auto results = solve(A, B, strategies, settings);
  EXPECT_NEAR(results(0), gt2[0], 1.0e-3);
  EXPECT_NEAR(results(1), gt2[1], 1.0e-3);
}

// Constant norm constraint
TEST(IrlsTest, ConstantNormConstraint) {
  using namespace irls;

  double oneOverSqrt2 = 1.0/sqrt(2.0);

  double tgt0[2] = {2.0, 2.0};
  double tgt1[2] = {-0.5, 0.5};

  double gt0[2] = {oneOverSqrt2, oneOverSqrt2};
  double gt1[2] = {-oneOverSqrt2, oneOverSqrt2};

  constantNormConstraint(tgt0, gt0);
  constantNormConstraint(tgt1, gt1);
}

TEST(IrlsTest, BoundConstrainedCurveFit) {
  // This type of constraint
  // is also used for GPS-filtering.

  using namespace irls;
  int n = 12;
  int middle = n/2;
  int difCount = n - 1;
  int fitElemCount = n;
  int difElemCount = 2*difCount;
  int rows = n + difCount;
  int cols = n;

  Eigen::VectorXd B(rows);
  Array<Triplet> triplets(fitElemCount + difElemCount);

  auto fits = triplets.sliceTo(fitElemCount);
  auto difs = triplets.sliceFrom(fitElemCount);
  for (int i = 0; i < n; i++) {
    fits[i] = Triplet(i, i, 1.0);
    B(i) = (i < middle? 0 : 3);
  }
  Array<Spani> boundedRowSpans(difCount);
  for (int i = 0; i < difCount; i++) {
    int row = n + i;
    int offset = 2*i;
    difs[offset + 0] = Triplet(row, i, 1.0);
    difs[offset + 1] = Triplet(row, i, -1.0);
    B(row) = 0.0;
    boundedRowSpans[i] = Spani(row, row + 1);
    std::cout << EXPR_AND_VAL_AS_STRING(row) << std::endl;

  }

  WeightingStrategies strategies{
    BoundedNormConstraint::make(boundedRowSpans, 1.0)
  };

  Eigen::SparseMatrix<double> A(rows, cols);
  A.setFromTriplets(triplets.begin(), triplets.end());

  Settings settings;
  settings.iters = 200;

  auto results = solve(A, B, strategies, settings);



  for (auto triplet: triplets) {
    std::cout << "Triplet (" << triplet.row() << ", " << triplet.col() << ", "
        << triplet.value() << ")" << std::endl;
  }
  std::cout << "A = \n" << A.toDense() << std::endl;
  std::cout << "B = \n" << B << std::endl;

  std::cout << "Curve: " << results << std::endl;
}
