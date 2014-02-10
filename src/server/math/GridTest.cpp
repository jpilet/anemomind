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

TEST(GridTest, LinearCombinationTest)
{
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
  for (int i = 0; i < vdim; i++)
  {
    wsum += weights[i];
  }
  assert(std::abs(wsum - 1) <= 1.0e-6);

  double y[2];
  grid.evalVertexLinearCombination(inds, weights, y);
  assert((norm2dif<double, 2>(x, y) <= 1.0e-6));
}

// Test the correctness of the imcompressability regularization with dimensions
//   X, Y, time
TEST(GridTest, IncompressTest3d)
{
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
TEST(GridTest, IncompressTest2d)
{
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





