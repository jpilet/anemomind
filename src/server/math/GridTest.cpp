/*
 * GridTest.cpp
 *
 *  Created on: 23 janv. 2014
 *      Author: jonas
 */

#include "gtest/gtest.h"
#include "../common/BBox.h"
#include "Grid.h"
#include "mathutils.h"

using namespace sail;

TEST(GridTest, LinearCombinationTest) {
  double lower[2] = {-0.1, -0.1};
  double upper[2] = {1.2, 1.2};

  double x[2] = {0.6, 0.6};
  double spacing[2] = {1.0, 1.0};

  BBox2d box;
  box.extend(lower);
  box.extend(upper);

  Grid2d grid(box, spacing);

  const int vdim = grid.WVL;
  int inds[vdim];
  double weights[vdim];
  grid.makeVertexLinearCombination(x, inds, weights);

  double wsum = 0.0;
  for (int i = 0; i < vdim; i++) {
    wsum += weights[i];
  }
  EXPECT_NEAR(wsum, 1.0, 1.0e-6);

  double y[2];
  grid.evalVertexLinearCombination(inds, weights, y);
  EXPECT_NEAR((norm2dif<double, 2>(x, y)), 0.0, 1.0e-6);
}

// Test the correctness of the imcompressability regularization with dimensions
//   X, Y, time
TEST(GridTest, IncompressTest3d) {
  double spacing[3] = {1.0, 1.0, 1.0};
  double minv[3] = {-0.1, -0.1, -0.1};
  double maxv[3] = {1.1, 1.1, 1.1};
  BBox3d bbox;
  bbox.extend(minv);
  bbox.extend(maxv);

  Grid3d grid(bbox, spacing);
  arma::sp_mat A = makeIncompressReg(grid);

  const int dimsPerVertex = 2; // At every vertex, there is a 2d flow
  int vertexCount = grid.getVertexCount();
  int Xdim = vertexCount*dimsPerVertex;
  EXPECT_TRUE(A.n_cols == Xdim);

  // This vector represents a homogenous flow over space and time of [1 1] at every vertex.
  arma::mat X = arma::ones(Xdim, 1);

  EXPECT_NEAR(arma::norm(A*X, 2), 0.0, 1.0e-6);
}

// The imcompressability for a 2d Grid, only X and Y but no time.
TEST(GridTest, IncompressTest2d) {
  double spacing[2] = {1.0, 1.0};
  double minv[2] = {-0.1, -0.1};
  double maxv[2] = {1.1, 1.1};
  BBox2d bbox;
  bbox.extend(minv);
  bbox.extend(maxv);

  Grid2d grid(bbox, spacing);
  arma::sp_mat A = makeIncompressReg(grid);
  const int dimsPerVertex = 2; // At every vertex, there is a 2d flow
  int vertexCount = grid.getVertexCount();
  int Xdim = vertexCount*dimsPerVertex;
  EXPECT_TRUE(A.n_cols == Xdim);
  // This vector represents a homogenous flow over space and time of [1 1] at every vertex.
  arma::mat X = arma::ones(Xdim, 1);
  EXPECT_NEAR(arma::norm(A*X, 2), 0.0, 1.0e-6);
}

TEST(GridTest, Filter3Test) {
  double spacing[1] = {1.0};
  double minv = 0.0;
  double maxv = 30.0;
  Grid1d grid(BBox1d(Spand(minv, maxv)), spacing);
  int n = grid.getVertexCount();
  EXPECT_LE(5, n);
  Arrayd src = Arrayd::fill(n, 0.0);
  Arrayd dst = Arrayd::fill(n, 119.0);
  src[1] = 1.0;
  double coefs[3] = {1, 1, 1};
  grid.filter3(src, dst, 0, coefs, true);
  const double marg = 1.0e-3;
  EXPECT_NEAR(dst[0], 1.0/3, marg);
  EXPECT_NEAR(dst[1], 1.0/3, marg);
  EXPECT_NEAR(dst[2], 1.0/3, marg);
  EXPECT_NEAR(dst[3], 0.0,   marg);
  EXPECT_NEAR(dst[n-1], 0.0, marg);
}


namespace {
  double evalDist(double *x2) {
    return sqr(x2[0] - 3.0) + sqr(x2[1] - 4.0);
  }
}

TEST(GridTest, Minimize) {
  Spand spans[2] = {Spand(-6, 6), Spand(-7, 7)};
  int inddata[2] = {13, 15};
  MDInds<2> inds(inddata);
  Grid<2> grid(inds, spans);
  double x[2] = {300, 300};
  EXPECT_NEAR(grid.minimize(&evalDist, x), 0.0, 1.0e-6);
  EXPECT_NEAR(x[0], 3.0, 1.0e-6);
  EXPECT_NEAR(x[1], 4.0, 1.0e-6);
}





