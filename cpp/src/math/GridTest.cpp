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

// The the imcompressability for a 2d Grid
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
	DOUT(MAKEDENSE(A));
	PFL;
	EXPECT_NEAR(arma::norm(A*X, 2), 0.0, 1.0e-6);
	PFL;
}





