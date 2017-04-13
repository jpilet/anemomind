/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/irls.h>
#include <gtest/gtest.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/math/nonlinear/DataFit.h>
#include <Eigen/Core>

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

TEST(IrlsTest, ConstraintGroup) {
  Spani a(0, 2);
  Spani b(2, 3);
  Arrayd residuals{3, 4, 9.8};
  double cstWeight = 15;
  irls::ConstraintGroup generalGroup(Array<Spani>{a, b}, 1);
  irls::BinaryConstraintGroup binaryGroup(a, b);
  irls::QuadCompiler gComp(3), bComp(3);
  generalGroup.apply(cstWeight, residuals, &gComp);
  binaryGroup.apply(cstWeight, residuals, &bComp);
  auto generalResults = gComp.makeWeightsAndOffset();
  auto binaryResults = bComp.makeWeightsAndOffset();
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(generalResults.weights.diagonal()(i), binaryResults.weights.diagonal()(i), 1.0e-9);

  }
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
      strategies, settings).X;
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


// Minimize (x + 2)^2 subject to x >= 0.3
// The solution to this is x = 0.3 and the constraint is active.
TEST(IrlsTest, NonNegCst1Active) {
  using namespace irls;
  Array<Triplet> triplets{Triplet(0, 0, 1.0), Triplet(1, 0, 1.0)};
  Eigen::SparseMatrix<double> A(2, 1);
  A.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::VectorXd B(2);
  B(0) = -2;
  B(1) = 0.3;
  Settings settings;
  settings.iters = 30;
  Eigen::VectorXd X = solve(A, B,
      WeightingStrategies{NonNegativeConstraint::make(1)}, settings).X;
  std::cout << EXPR_AND_VAL_AS_STRING(X[0]) << std::endl;
  EXPECT_NEAR(X[0], 0.3, 1.0e-2);
}

// Minimize (x - 2)^2 subject to x >= 0.3
// The solution to this is x = 2 and the constraint is passive.
TEST(IrlsTest, NonNegCst2Passive) {
  using namespace irls;
  Array<Triplet> triplets{Triplet(0, 0, 1.0), Triplet(1, 0, 1.0)};
  Eigen::SparseMatrix<double> A(2, 1);
  A.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::VectorXd B(2);
  B(0) = 2;
  B(1) = 0.3;
  Settings settings;
  settings.iters = 12;
  Eigen::VectorXd X = solve(A, B,
      WeightingStrategies{NonNegativeConstraint::make(1)}, settings).X;
  EXPECT_NEAR(X[0], 2.0, 1.0e-3);
}

// Minimize (x0 - 1)^2 + (x1 - 1)^2 subject to x0 < 0.4 and x1 < 0.6
// The solution is x0 = 0.4 and x1 = 0.6
TEST(IrlsTest, NonNegCorner) {
  using namespace irls;
  Array<Triplet> triplets(4);
  Eigen::VectorXd B(4);
  for (int i = 0; i < 4; i++) {
    triplets[i] = Triplet(i, i % 2, (i < 2? 1.0 : -1.0));
  }
  Eigen::SparseMatrix<double> A(4, 2);
  A.setFromTriplets(triplets.begin(), triplets.end());
  B(0) = 1.0;
  B(1) = 1.0;
  B(2) = -0.4;
  B(3) = -0.6;
  Settings settings;
  Eigen::VectorXd X = solve(A, B,
      WeightingStrategies{NonNegativeConstraint::make(2), NonNegativeConstraint::make(3)}, settings).X;
  EXPECT_NEAR(X[0], 0.4, 1.0e-3);
  EXPECT_NEAR(X[1], 0.6, 1.0e-3);
}

// Minimize (x0 - 1)^2 + (x1 - 0.1)^2 subject to x0 < 0.4 and x1 < 0.6
// The solution is x0 = 0.4 and x1 = 0.1
TEST(IrlsTest, NonNegCorner2) {
  using namespace irls;
  Array<Triplet> triplets(4);
  Eigen::VectorXd B(4);
  for (int i = 0; i < 4; i++) {
    triplets[i] = Triplet(i, i % 2, (i < 2? 1.0 : -1.0));
  }
  Eigen::SparseMatrix<double> A(4, 2);
  A.setFromTriplets(triplets.begin(), triplets.end());
  B(0) = 1.0;
  B(1) = 0.1;
  B(2) = -0.4;
  B(3) = -0.6;
  Settings settings;
  Eigen::VectorXd X = solve(A, B,
      WeightingStrategies{NonNegativeConstraint::make(2), NonNegativeConstraint::make(3)}, settings).X;
  EXPECT_NEAR(X[0], 0.4, 1.0e-3);
  EXPECT_NEAR(X[1], 0.1, 1.0e-3);
}

// Minimize (x0 + 1)^2 + x1^2 subject to x0 + x1 >= 1
// The solution is x0 = 0 and x1 = 1
TEST(IrlsTest, NonNegSlope) {
  using namespace irls;
  Array<Triplet> triplets{Triplet(0, 0, 1.0), Triplet(1, 1, 1.0), Triplet(2, 0, 1.0), Triplet(2, 1, 1.0)};
  Eigen::VectorXd B(3);
  Eigen::SparseMatrix<double> A(3, 2);
  A.setFromTriplets(triplets.begin(), triplets.end());
  B(0) = -1.0;
  B(1) = 0.0;
  B(2) = 1.0;
  Settings settings;
  Eigen::VectorXd X = solve(A, B,
      WeightingStrategies{NonNegativeConstraint::make(2)}, settings).X;
  EXPECT_NEAR(X[0], 0.0, 1.0e-3);
  EXPECT_NEAR(X[1], 1.0, 1.0e-3);
}

// Minimize (x0 - 1)^2 + (x1 - 1)^2 subject to either x0 < 0.6 or x1 < 0.4
// The solution to this is x0 = 0.6 and x1 = 1
//
// To implement this problem, we introduce two slack variables, s0 and s1,
// and rewrite the problem to:
//
// Minimize (x0 - 1)^2 + (x1 - 1)^2 subject to
//  x0 - s0 < 0.6 and x1 - s1 < 0.4 and
// either s0=0 or s1=0
//
// So that in fact, both constraints are always used, but one of the
// slack variables must be 0, whereas the other slack variable can
// be anything in order to satisfy the constraint.
//
// This example is interesting, because it tests
// the interaction between the inequality constraints
// and another type of constraint.
TEST(IrlsTest, NonNegWithBinary) {
  using namespace irls;
  Array<Triplet> triplets{
    Triplet(0, 0, 1.0), Triplet(1, 1, 1.0),
    Triplet(2, 2, 1.0), Triplet(3, 3, 1.0),

    Triplet(4, 0, -1.0), Triplet(5, 1, -1.0),
    Triplet(4, 2, 1.0), Triplet(5, 3, 1.0)};
  Eigen::SparseMatrix<double> A(6, 4);
  A.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::VectorXd B(6);
  B(0) = 1;
  B(1) = 1;
  B(2) = 0;
  B(3) = 0;
  B(4) = -0.6;
  B(5) = -0.4;
  Settings settings;
  Eigen::VectorXd X = solve(A, B,
        WeightingStrategies{
          ConstraintGroup::make(Array<Spani>{Spani(2, 3), Spani(3, 4)}, 1),
          NonNegativeConstraint::make(4),
          NonNegativeConstraint::make(5)}, settings).X;
  EXPECT_NEAR(X(0), 0.6, 0.0001);
  EXPECT_NEAR(X(1), 1.0, 0.0001);
}

TEST(IrlsTest, Constant) {
  using namespace irls;
  Array<Triplet> triplets{Triplet(0, 0, 1.0)};
  Eigen::SparseMatrix<double> A(1, 1);
  A.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::VectorXd B(1);
  B(0) = 0.0;
  Settings settings;
  auto X = solve(A, B,
      WeightingStrategies{Constant::make(0, MajQuad::fit(2.3))},
      settings).X;
  EXPECT_NEAR(X[0], 2.3, 1.0e-6);
}

MDArray2d makeTargetSpeedTestData(int n, int outlierPeriod) {
  int counter = 0;
  MDArray2d dst(n, 2);
  for (int i = 0; i < n; i++) {
    // Poor man's RNG. Just so that we are sure
    // to get the same numbers on any machine :-)
    double x = 0.5*(1 + sin(exp(0.34*i)));
    double y = 0.5*(1 + sin(exp(0.4*i)));
    if ((y < x || i % outlierPeriod == 0) && std::isfinite(x) && std::isfinite(y)) {
      dst(counter, 0) = x;
      if (y < x) {
        double lambda = 1.0 - y/x;
        lambda *= lambda;
        lambda *= lambda;
        dst(counter, 1) = x*(1.0 - lambda);
      } else {
        dst(counter, 1) = y;
      }
      counter++;
    }
  }
  return dst.sliceRowsTo(counter);
}

using namespace DataFit;



std::function<void(Triplet)> makeTripletAdder(int rows, int cols, std::vector<Triplet> *dst) {
  Spani rowSpan(0, rows);
  Spani colSpan(0, cols);
  return [=](Triplet t) {
    CHECK(rowSpan.contains(t.row()));
    CHECK(colSpan.contains(t.col()));
    dst->push_back(t);
  };
}
typedef std::function<void(Triplet)> TripletAdder;

irls::WeightingStrategy::Ptr makeGravity(CoordIndexer gravityRows,
    CoordIndexer vertexCols, TripletAdder adder) {

  auto slope = MajQuad(0, 1) + 1.0e-6*MajQuad(1.0, 0.0);

  int n = gravityRows.numel();
  Arrayi inds(n);
  assert(n == vertexCols.numel());
  for (int i = 0; i < n; i++) {
    inds[i] = gravityRows[i];
    adder(Triplet(gravityRows[i], vertexCols[i], 1.0));
  }
  return irls::Constant::make(inds, slope);
}

typedef irls::NonNegativeConstraint NNCst;

irls::WeightingStrategy::Ptr makeDataSupport(LineKM vertexIndexToCoord,
    MDArray2d data,
    CoordIndexer dataSupportRows, CoordIndexer vertexCols,
    CoordIndexer slackCols, TripletAdder adder,
    Eigen::VectorXd *B) {
  int n = data.rows();
  Arrayi inds(n);
  for (int i = 0; i < n; i++) {
    double x = data(i, 0);
    double raw = vertexIndexToCoord.inv(x);
    int lower = int(floor(raw));
    int col = vertexCols[lower];
    double lambda = raw - lower;
    int row = dataSupportRows[i];
    adder(Triplet(row, col, 1.0 - lambda));
    adder(Triplet(row, col+1, lambda));
    adder(Triplet(row, slackCols[i], 1.0));
    inds[i] = row;
    (*B)[row] = data(i, 1);
  }
  return NNCst::make(inds);
}

irls::WeightingStrategy::Ptr makeInlierSlack(CoordIndexer inlierRows,
  CoordIndexer slackCols, int inlierCount, TripletAdder adder) {
  int n = inlierRows.numel();
  Array<Spani> spans(n);
  for (int i = 0; i < n; i++) {
    int row = inlierRows[i];
    adder(Triplet(row, slackCols[i], 1.0));
    spans[i] = Spani(row, row + 1);
  }
  return irls::ConstraintGroup::make(spans, inlierCount);
}

irls::WeightingStrategy::Ptr makeMonotonyConstraints(CoordIndexer monotonyRows,
    CoordIndexer vertexCols, TripletAdder adder) {
    CHECK(monotonyRows.numel() == vertexCols.numel());
    int n = monotonyRows.numel();
    Arrayi inds(n);
    for (int i = 0; i < n; i++) {
      int row = monotonyRows[i];
      inds[i] = row;
      int col = vertexCols[i];
      if (i < n-1) {
        adder(Triplet(row, col, -1.0));
        adder(Triplet(row, col+1, 1.0));
      } else {
        adder(Triplet(row, 0, 1.0));
      }
    }
  return NNCst::make(inds);
}


bool validTriplets(int rows, int cols, std::vector<Triplet> triplets) {
  Spani rowSpan(0, rows);
  Spani colSpan(0, cols);
  for (auto t: triplets) {
    if (!rowSpan.contains(t.row())) {
      LOG(WARNING) << "Bad row: " << t.row();
      return false;
    }
    if (!colSpan.contains(t.col())) {
      LOG(WARNING) << "Bad col: " << t.col();
      return false;
    }
  }
  return true;
}



/*
 * Explanation:
 * This algorithm identifies a subset of all
 * target speed data points to use as inliers, such
 * that the height of every vertex of the target speed surface
 * is minimum subject to the constraint that the target speed surface should
 * rest on the inlier points. Also the target speed is constrained
 * not to decrease as the wind increases.
 *
 * The idea is to use this approach, but to compute polars.
 */
void targetSpeedPrototype(bool visualize, int iters) {
  auto data = makeTargetSpeedTestData(4000, 8);

  double percentile = 0.88;

  int inlierCount = int(round(percentile*data.rows()));

  int vertexCount = 12;
  LineKM vertexIndexToCoord(0, vertexCount-1, 0.0, 1.0);

  DataFit::CoordIndexer::Factory rows;
  auto gravityRows = rows.make(vertexCount, 1);
  auto dataSupportRows = rows.make(data.rows(), 1);
  auto inlierRows = rows.make(data.rows(), 1);
  auto monotonyRows = rows.make(vertexCount, 1);
  DataFit::CoordIndexer::Factory cols;
  auto vertexCols = cols.make(vertexCount, 1);
  auto slackCols = cols.make(data.rows(), 1);

  std::vector<Triplet> triplets;

  Eigen::SparseMatrix<double> A(rows.count(), cols.count());
  Eigen::VectorXd B = Eigen::VectorXd::Zero(rows.count());
  auto adder = makeTripletAdder(rows.count(), cols.count(), &triplets);

  // This part just tries to minimize the vertices
  LOG(INFO) << "gravity";
  auto gravityWeighting = makeGravity(gravityRows, vertexCols, adder);

  // This part forces the surface to rest on top of the points
  // (except for the outliers)
  LOG(INFO) << "data";
  auto supportConstraints = makeDataSupport(vertexIndexToCoord, data,
      dataSupportRows, vertexCols, slackCols, adder, &B);

  // This part identifies the inliers
  LOG(INFO) << "slack";
  auto inlierWeighting = makeInlierSlack(inlierRows,
    slackCols, inlierCount, adder);

  LOG(INFO) << "monotony";
  // This part forces the target speed to increase as the wind increases.
  auto monotonyConstraints = makeMonotonyConstraints(monotonyRows, vertexCols, adder);
  CHECK(validTriplets(rows.count(), cols.count(), triplets));
  A.setFromTriplets(triplets.begin(), triplets.end());

  irls::Settings settings;
  settings.iters = iters;
  auto results = irls::solve(A, B, irls::WeightingStrategies{
    gravityWeighting, supportConstraints,
    inlierWeighting,
    monotonyConstraints
  }, settings);
  auto X = results.X;


  Eigen::VectorXd vertices = X.block(0, 0, vertexCount, 1);

  MDArray2d vertexData(vertexCount, 2);
  for (int i = 0; i < vertexCount; i++) {
    auto x = vertexIndexToCoord(i);
    auto y = vertices(i);
    EXPECT_NEAR(x, y, 0.05);

    vertexData(i, 0) = x;
    vertexData(i, 1) = y;
  }

  if (visualize) {
    GnuplotExtra plot;
    plot.plot(data);
    plot.set_style("lines");
    plot.plot(vertexData);
    plot.show();
  }
}

TEST(IrlsTest, TargetSpeedPrototype) {
  bool visualize = false;
  targetSpeedPrototype(visualize, 30);
}
