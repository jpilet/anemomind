/*
 * Grid.cpp
 *
 *  Created on: 23 janv. 2014
 *      Author: jonas
 */

#include "Grid.h"
#include <armadillo>

namespace sail {

namespace {
void setSpElement(arma::umat *IJOut, arma::vec *XOut, int index, int i, int j, double x) {
  (*IJOut)(0, index) = i;
  (*IJOut)(1, index) = j;
  (*XOut)[index] = x;
}


// Output the four elements representing the imcompressability of a single cell in the grid
template <int N>
void outputIncompressEquation(int row, const MDInds<N> &inds, int *indexArray,
                              arma::umat *IJOut, arma::vec *XOut) {
  int ioffs = indexArray[0];
  int joffs = indexArray[1];

  // Loop over the four corners of this cell
  int at = 8*row;
  for (int i = 0; i < 2; i++) {
    indexArray[0] = ioffs + i;
    int ysgn = 2*i - 1;
    for (int j = 0; j < 2; j++) {
      int xsgn = 1 - 2*j;
      indexArray[1] = joffs + j;
      assert(inds.valid(indexArray));
      int vertexIndex = inds.calcIndex(indexArray);
      int coloffs = 2*vertexIndex;

      setSpElement(IJOut, XOut, at, row, coloffs, xsgn);
      setSpElement(IJOut, XOut, at + 1, row, coloffs + 1, ysgn);

      at += 2; // for x and y components
    }
  }
}

template <int N>
void makeIncompressRegSub(int rowoffset, const MDInds<N> &inds, int *indexArray, arma::umat *IJOut, arma::vec *XOut) {
  const int W = inds.get(0) - 1;
  const int H = inds.get(1) - 1;
  int counter = 0;
  for (int i = 0; i < W; i++) {
    for (int j = 0; j < H; j++) {
      indexArray[0] = i;
      indexArray[1] = j;
      outputIncompressEquation<N>(rowoffset + counter, inds, indexArray, IJOut, XOut);
      counter += 1;
    }
  }
  assert(counter == W*H);
}
}

arma::sp_mat makeIncompressReg(Grid3d grid) {
  const MDInds<3> &inds = grid.getInds();
  int levels = inds.get(2);
  int cellsPerLevel = (inds.get(0) - 1)*(inds.get(1) - 1);
  const int elemsPerCell = 8;
  int eqCount = cellsPerLevel*levels;
  int elems = elemsPerCell*eqCount;
  int cells = cellsPerLevel*levels;
  int elemsPerLevel = elemsPerCell*cellsPerLevel;
  arma::umat IJ(2, elems);
  arma::vec X(elems);
  int rowoffset = 0;
  for (int i = 0; i < levels; i++) {
    int indexArray[3] = {0, 0, i};
    makeIncompressRegSub<3>(rowoffset, inds, indexArray, &IJ, &X);

    rowoffset += cellsPerLevel;
  }
  assert(rowoffset == cells);
  return arma::sp_mat(IJ, X, eqCount, 2*grid.getVertexCount());
}

arma::sp_mat makeIncompressReg(Grid2d grid) {
  const MDInds<2> &inds = grid.getInds();
  int indexArray[2] = {0, 0};
  int eqCount = (inds.get(0) - 1)*(inds.get(1) - 1);
  int elems = 8*eqCount;
  arma::umat IJ(2, elems);
  arma::vec X(elems);
  makeIncompressRegSub<2>(0, inds, indexArray, &IJ, &X);
  return arma::sp_mat(IJ, X, eqCount, 2*grid.getVertexCount());
}



}

