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
#include <server/math/irlsFixedDenseBlock.h>

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

TEST(IrlsTest, ArrayView) {
  MDArray2d A(9, 2);
  int counter = 0;
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 2; j++) {
      counter++;
      A(i, j) = counter;
    }
  }

  auto view = irls::arrayView(&A, 3, 2, 4);
  EXPECT_EQ(view.rows(), 3);
  EXPECT_EQ(view.cols(), 2);

  EXPECT_EQ(view(0, 0), 9);
  EXPECT_EQ(view(1, 1), 12);
  EXPECT_EQ(view(2, 0), 13);

  view(2, 1) += 20;

  EXPECT_EQ(A(6, 1), 34);
}

TEST(IrlsTest, BandMatrixView) {
  auto mat = BandMatrix<double>::zero(5, 5, 1, 1);

  mat(2, 2) = 1.0;
  mat(2, 3) = 2.0;
  mat(3, 2) = 3.0;
  mat(3, 3) = 4.0;

  auto view = irls::bandMatrixView(&mat, 2, 2);

  EXPECT_EQ(view.rows(), 2);
  EXPECT_EQ(view.cols(), 2);

  EXPECT_EQ(1.0, view(0, 0));
  EXPECT_EQ(2.0, view(0, 1));
  EXPECT_EQ(3.0, view(1, 0));
  EXPECT_EQ(4.0, view(1, 1));

  view += 3.0*Eigen::MatrixXd::Identity(2, 2);

  EXPECT_NEAR(view(1, 1), 7.0, 1.0e-6);
}

TEST(IrlsTest, FixedDenseBlock) {
  typedef irls::FixedDenseBlock<3, 2, 1> Block;
  Block::AType A;
  A << 1, 2,
       3, 4,
       5, 6;
  Block::BType B;
  B << 3,
       -1,
       4;

  Eigen::VectorXd weights(11);
  weights << 8,
             6,
             1,
             6,
             3,
             2,
             8,
             1,
             6,
             1,
             7;

  Eigen::VectorXd X(5);
  X << 9,
       6,
       9,
       6,
       4;

  Block block(A, B, 2/*row*/, 1/*col*/);

  EXPECT_EQ(block.lhsCols(), 2);
  EXPECT_EQ(block.rhsCols(), 1);

  //// Test eval

  Eigen::VectorXd residuals = Eigen::VectorXd::Zero(11);

  // The subvector of X that we use starts at 1.
  block.eval(X, &residuals);

  // row = 2, so thats where we offset
  EXPECT_EQ(residuals(1), 0);
  EXPECT_EQ(residuals(2), 21);
  EXPECT_EQ(residuals(3), 55);
  EXPECT_EQ(residuals(4), 80);
  EXPECT_EQ(residuals(5), 0);



  //// TEST accumulate
  BandMatrix<double> AtA(5, 5, 1, 1);
  AtA.setAll(2.4);

  MDArray2d AtB(5, 1);
  AtB.setAll(3.0);

  // (the weighting vector is (1, 6, 3) which is the slice starting from row=2.
  block.accumulateWeighted(weights, &AtA, &AtB);

  EXPECT_NEAR(AtA(0, 0), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(1, 0), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(0, 1), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(1, 1), 552.40, 1.0e-6);
  EXPECT_NEAR(AtA(2, 2), 906.40, 1.0e-6);
  EXPECT_NEAR(AtA(1, 2), 706.40, 1.0e-6);
  EXPECT_NEAR(AtA(2, 1), 706.40, 1.0e-6);
  EXPECT_NEAR(AtA(3, 3), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(3, 4), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(4, 3), 2.4, 1.0e-6);

}


TEST(IrlsTest, FixedDenseBlockMultiRHS) {
  typedef irls::FixedDenseBlock<3, 2, 2> Block;
  Block::AType A;
  A << 1, 2,
       3, 4,
       5, 6;
  Block::BType B;
  B << 3, 2,
       -1, 1,
       4, 2;

  Eigen::VectorXd weights(11);
  weights << 8,
             6,
             1,
             6,
             3,
             2,
             8,
             1,
             6,
             1,
             7;

  Eigen::MatrixXd X(5, 2);
  X << 9, 1,
       6, 1,
       9, 2,
       6, 3,
       4, 4;

  Block block(A, B, 2/*row*/, 1/*col*/);

  EXPECT_EQ(block.lhsCols(), 2);
  EXPECT_EQ(block.rhsCols(), 2);
  EXPECT_EQ(block.minDiagWidth(), 1);
  EXPECT_EQ(block.requiredRows(), 2 + 3);
  EXPECT_EQ(block.requiredCols(), 1 + 2);

  //// Test eval

  Eigen::VectorXd residuals = Eigen::VectorXd::Zero(11);

  // The subvector of X that we use starts at 1, that is (6, 9)
  block.eval(X, &residuals);

  // row = 2, so thats where we offset
  EXPECT_EQ(residuals(1), 0);
  EXPECT_NEAR(residuals(2), 21.213, 0.01);
  EXPECT_NEAR(residuals(3), 55.902, 0.01);
  EXPECT_NEAR(residuals(4), 81.394, 0.01);
  EXPECT_EQ(residuals(5), 0);




  //// TEST accumulate
  BandMatrix<double> AtA(5, 5, 1, 1);
  AtA.setAll(2.4);

  MDArray2d AtB(5, 2);
  AtB.setAll(3.0);

  // (the weighting vector is (1, 6, 3) which is the slice starting from row=2.
  block.accumulateWeighted(weights, &AtA, &AtB);

  EXPECT_NEAR(AtA(0, 0), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(1, 0), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(0, 1), 2.4, 1.0e-6);

  EXPECT_NEAR(AtA(1, 1), 552.40, 1.0e-6);
  EXPECT_NEAR(AtA(2, 2), 906.40, 1.0e-6);
  EXPECT_NEAR(AtA(1, 2), 706.40, 1.0e-6);
  EXPECT_NEAR(AtA(2, 1), 706.40, 1.0e-6);

  EXPECT_NEAR(AtA(3, 3), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(3, 4), 2.4, 1.0e-6);
  EXPECT_NEAR(AtA(4, 3), 2.4, 1.0e-6);

  EXPECT_NEAR(AtB(1, 0), 78.0, 1.0e-6);
  EXPECT_NEAR(AtB(1, 1), 203.0, 1.0e-6);
}

namespace {
  typedef irls::FixedDenseBlock<1, 1, 1> Block111;

  irls::DenseBlock::Ptr blk111(int row, int col, double a, double b) {
    Block111::AType A;
    A(0, 0) = a;
    Block111::BType B;
    B(0, 0) = b;
    return irls::DenseBlock::Ptr(new Block111(A, B, row, col));
  }
}

TEST(IrlsTest, MiniSolveTest) {
  int rows = 3;
  int cols = 3;
  Array<irls::DenseBlock::Ptr> blocks{
    blk111(0, 0, 1, 4), blk111(1, 1, 2, 3), blk111(2, 2, 3, 2)
  };

  irls::Settings settings;
  irls::ResultsMat solution = irls::solveBanded(rows, cols, blocks,
      Array<irls::WeightingStrategy::Ptr>(), settings);

  EXPECT_EQ(solution.X.rows(), 3);
  EXPECT_EQ(solution.X.cols(), 1);

  EXPECT_NEAR(solution.X(0, 0), 4.0, 1.0e-6);
  EXPECT_NEAR(solution.X(1, 0), 1.5, 1.0e-6);
  EXPECT_NEAR(solution.X(2, 0), 2.0/3, 1.0e-6);

}


namespace {
  typedef irls::FixedDenseBlock<1, 1, 2> Block112;

  irls::DenseBlock::Ptr blk112(int row, int col, double a, double b0, double b1) {
    Block112::AType A;
    A(0, 0) = a;
    Block112::BType B;
    B(0, 0) = b0;
    B(0, 1) = b1;
    return irls::DenseBlock::Ptr(new Block112(A, B, row, col));
  }

  class WeighBy0 : public irls::WeightingStrategy {
  public:
    WeighBy0(int index) : _index(index) {}

    void apply(
        double constraintWeight,
        const Arrayd &residuals, irls::QuadCompiler *dst) {
      dst->setWeight(_index, 0.0);
    }

    void initialize(const irls::Settings &s, irls::QuadCompiler *dst) {
      dst->setWeight(_index, 0.0);
    }
  private:
    int _index;
  };
}

TEST(IrlsTest, MiniSolveTest2) {
  int rows = 4;
  int cols = 3;

  Array<irls::DenseBlock::Ptr> blocks{
    blk112(0, 0, 1, 4.0, 5.0),
    blk112(1, 1, 2, 3.0, 7.0),
    blk112(2, 2, 3, 2.0, 1.0),
    blk112(3, 2, 1013, 32443.44, 132.34)
  };

  BandMatrix<double> AtA(3, 3, 2, 2);
  AtA.setAll(0.0);
  MDArray2d AtB(3, 2);
  AtB.setAll(0.0);

  Eigen::VectorXd weights = Eigen::VectorXd::Ones(3);

  irls::Settings settings;
  irls::ResultsMat solution = irls::solveBanded(rows, cols, blocks,
      Array<irls::WeightingStrategy::Ptr>{
    irls::WeightingStrategy::Ptr(new WeighBy0(3))
  }, settings);

  EXPECT_EQ(solution.X.rows(), 3);
  EXPECT_EQ(solution.X.cols(), 2);

  EXPECT_NEAR(solution.X(0, 0), 4.0, 1.0e-6);
  EXPECT_NEAR(solution.X(1, 0), 1.5, 1.0e-6);
  EXPECT_NEAR(solution.X(2, 0), 2.0/3, 1.0e-6);

  EXPECT_NEAR(solution.X(0, 1), 5.0, 1.0e-6);
  EXPECT_NEAR(solution.X(1, 1), 3.5, 1.0e-6);
  EXPECT_NEAR(solution.X(2, 1), 1.0/3, 1.0e-6);
}

/*
 * Given the constraint that we should be inside a circle located at (2, 1) with radius 0.5,
 * minimize the distance to the point at (1, 0). In that case, the constraint will be ACTIVE.
 */
TEST(IrlsTest, BoundedNorm) {
  using namespace irls;
  int rows = 4;
  int cols = 2;
  Array<Triplet> triplets{
    Triplet(0, 0, 1.0), Triplet(1, 1, 1.0),
    Triplet(2, 0, 1.0), Triplet(3, 1, 1.0)
  };
  Eigen::VectorXd B = Eigen::VectorXd::Zero(4);
  B(0) = 1.0;
  B(1) = 0.0;
  B(2) = 2.0;
  B(3) = 1.0;

  Eigen::SparseMatrix<double> A(rows, cols);
  A.setFromTriplets(triplets.begin(), triplets.end());

  double radius = 0.5;
  double expectedX = 2.0 - radius/sqrt(2.0);
  double expectedY = 1.0 - radius/sqrt(2.0);

  Array<WeightingStrategy::Ptr> strategies{
    WeightingStrategy::Ptr(new BoundedNormConstraint(Spani(2, 4), radius))
  };
  Settings settings;
  auto results = irls::solve(A, B, strategies, settings);
  EXPECT_EQ(results.X.rows(), 2);
  EXPECT_NEAR(results.X(0), expectedX, 1.0e-5);
  EXPECT_NEAR(results.X(1), expectedY, 1.0e-5);
}

/*
 * Given the constraint that we should be inside a circle located at (2, 1) with radius 0.5,
 * minimize the distance to the point at (2.1, 1.2). In that case, the constraint will be PASSIVE.
 */
TEST(IrlsTest, BoundedNorm2) {
  using namespace irls;
  int rows = 4;
  int cols = 2;
  Array<Triplet> triplets{
    Triplet(0, 0, 1.0), Triplet(1, 1, 1.0),
    Triplet(2, 0, 1.0), Triplet(3, 1, 1.0)
  };
  Eigen::VectorXd B = Eigen::VectorXd::Zero(4);
  B(0) = 2.1;
  B(1) = 1.2;
  B(2) = 2.0;
  B(3) = 1.0;

  Eigen::SparseMatrix<double> A(rows, cols);
  A.setFromTriplets(triplets.begin(), triplets.end());

  double radius = 0.5;
  double expectedX = 2.1;
  double expectedY = 1.2;

  Array<WeightingStrategy::Ptr> strategies{
    WeightingStrategy::Ptr(new BoundedNormConstraint(Spani(2, 4), radius))
  };
  Settings settings;
  auto results = irls::solve(A, B, strategies, settings);
  EXPECT_EQ(results.X.rows(), 2);
  EXPECT_NEAR(results.X(0), expectedX, 1.0e-5);
  EXPECT_NEAR(results.X(1), expectedY, 1.0e-5);
}

