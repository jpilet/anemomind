/*
 * Grid.h
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#ifndef GRID_H_
#define GRID_H_

#include <armadillo>
#include <server/common/BBox.h>
#include <server/common/LineKM.h>
#include <server/common/MDArray.h>
#include <server/common/MDInds.h>
#include <server/common/math.h>

namespace sail {

// Represents a grid with N dimensions
template <int N>
class Grid {
 public:
  typedef Grid<N> ThisType;


  Grid() {}
  Grid(BBox<N> bbox, double *spacingN) {
    initialize(bbox, spacingN);
  }

  Grid(Span s, double spacing) {
    static_assert(N == 1, "This constructor is only applicable to one dimensional grids.");
    BBox<N> bbox(s);
    initialize(bbox, &spacing);
  }

  virtual ~Grid() {}

  LineKM &getEq(int dim) {
    return _ind2Coord[dim];
  }
  int getSize(int dim) {
    return _inds.get(dim);
  }

  MDArray2d getGridVertexCoords() {
    int count = _inds.numel();
    MDArray2d pts(count, N);

    for (int i = 0; i < count; i++) {
      int I[N];
      _inds.calcInv(i, I);
      for (int j = 0; j < N; j++) {
        pts(i, j) = _ind2Coord[j](I[j]);
      }
    }
    return pts;
  }



  // Number of vertices in a linear combination
  static const int WVL = StaticPower<2, N>::result;

  // Expresses a point as a linear combination of the grid vertices
  void makeVertexLinearCombination(double *vecN, int *indsOut, double *weightsOut) {
    int indsFloor[N];
    double lambda[N];
    int tmpSize[N];
    for (int i = 0; i < N; i++) {
      double x = _ind2Coord[i].inv(vecN[i]); // Compute a floating point position in the grids inner coordinate system
      int I = int(x);
      indsFloor[i] = I;
      tmpSize[i] = 2;
      lambda[i] = x - I;
    }
    MDInds<N> iter(tmpSize);
    assert(iter.numel() == WVL);
    for (int i = 0; i < WVL; i++) {
      int K[N];
      iter.calcInv(i, K);
      int vertexInds[N];
      double weight = 1.0;
      for (int j = 0; j < N; j++) {
        int k = K[j];
        weight *= (k == 0? 1.0 - lambda[j] : lambda[j]);
        vertexInds[j] = indsFloor[j] + k;
      }
      weightsOut[i] = weight;
      indsOut[i] = _inds.calcIndex(vertexInds);
    }
  }

  void evalVertexLinearCombination(int *inds, double *weights, double *vecNOut) {
    for (int i = 0; i < N; i++) {
      vecNOut[i] = 0.0;
    }
    for (int i = 0; i < WVL; i++) {
      int I[N];
      double w = weights[i];
      _inds.calcInv(inds[i], I);
      for (int j = 0; j < N; j++) {
        vecNOut[j] += w*_ind2Coord[j](I[j]);
      }
    }
  }

  // Constructs a sparse regularization matrix
  // to encourage neighbouring values x, y along some
  // dimension to be similar, i.e. |x - y|² is penalized.
  arma::sp_mat makeFirstOrderReg(int dim) {
    assert(0 <= dim);
    assert(dim < N);
    MDInds<N> temp = _inds;
    temp.set(dim, temp.get(dim) - 1);
    int rows = temp.numel();
    int elemCount = 2*rows;
    int cols = _inds.numel();

    arma::umat IJ(2, elemCount); // Element locations in the matrix
    arma::vec X(elemCount);	 // Element values in the matrix
    for (int i = 0; i < rows; i++) {
      int offset = 2*i;
      int at[N];
      temp.calcInv(i, at);

      int I = _inds.calcIndex(at);
      at[dim]++;
      int J = _inds.calcIndex(at);
      IJ(0, offset + 0) = i; // row
      IJ(1, offset + 0) = I; // col
      X[offset + 0] = 1.0;   // value

      IJ(0, offset + 1) = i; // row
      IJ(1, offset + 1) = J; // col
      X[offset + 1] = -1.0;  // value
    }
    return arma::sp_mat(IJ, X, rows, cols);
  }

  // Constructs a sparse regularization matrix
  arma::sp_mat makeSecondOrderReg(int dim) {
    assert(0 <= dim);
    assert(dim < N);
    MDInds<N> temp = _inds;
    temp.set(dim, temp.get(dim) - 2);
    int rows = temp.numel();
    const int blksize = 3;
    int elemCount = blksize*rows;
    int cols = _inds.numel();

    arma::umat IJ(2, elemCount); // Element locations in the matrix
    arma::vec X(elemCount);	 // Element values in the matrix
    for (int i = 0; i < rows; i++) {
      int offset = blksize*i;
      int at[N];
      temp.calcInv(i, at);

      int I = _inds.calcIndex(at);
      at[dim]++;
      int J = _inds.calcIndex(at);
      at[dim]++;
      int K = _inds.calcIndex(at);
      IJ(0, offset + 0) = i; // row
      IJ(1, offset + 0) = I; // col
      X[offset + 0] = 1.0;   // value

      IJ(0, offset + 1) = i; // row
      IJ(1, offset + 1) = J; // col
      X[offset + 1] = -2.0;  // value

      IJ(0, offset + 2) = i; // row
      IJ(1, offset + 2) = K; // col
      X[offset + 2] = 1.0;  // value
    }
    return arma::sp_mat(IJ, X, rows, cols);
  }

  // Computes a sparse matrix P such that P*X = ptsAsRows, X being the grid vertices.
  arma::sp_mat makeP(MDArray2d ptsAsRows) {
    assert(ptsAsRows.cols() == N);
    int count = ptsAsRows.rows();
    int elemCount = WVL*count;
    int rows = count;
    int cols = _inds.numel();
    arma::umat IJ(2, elemCount);
    arma::vec X(elemCount);

    double temp[N];
    MDArray2d temparr(1, N, temp);
    for (int i = 0; i < count; i++) {
      ptsAsRows.sliceRow(i).copyToSafe(temparr);
      int inds[WVL];
      double weights[WVL];
      makeVertexLinearCombination(temp, inds, weights);
      int offset = i*WVL;
      for (int j = 0; j < WVL; j++) {
        int index = offset + j;
        IJ(0, index) = i;		// row
        IJ(1, index) = inds[j]; // cols
        X[index] = weights[j];  // value
      }
    }

    return arma::sp_mat(IJ, X, rows, cols);
  }

  int getVertexCount() {
    return _inds.numel();
  }

  int cellCount() {
    MDInds<N> cellInds = _inds - 1;
    return cellInds.numel();
  }

  const MDInds<N> &getInds() const {
    return _inds;
  }
 private:
  MDInds<N> _inds;      // Holds the size of every
  LineKM _ind2Coord[N]; // Maps indices along a dimension to coordinates

  void initialize(BBox<N> bbox, double *spacingN) {
    int sizes[N];
    for (int i = 0; i < N; i++) {
      double &s = spacingN[i];
      Span &span = bbox.getSpan(i);
      int from = int(floor(span.getMinv()/s));
      int to = int(ceil(span.getMaxv()/s)) + 1;
      sizes[i] = to - from;
      _ind2Coord[i] = LineKM(0.0, sizes[i], from*s, to*s);
    }
    _inds = MDInds<N>(sizes);
  }
};

template <int N>
std::ostream &operator<<(std::ostream &s, Grid<N> grid) {
  s << "Grid[" << N << "]" << std::endl;
  s << "  Sizes: ";
  for (int i = 0; i < N; i++) {
    s << grid.getSize(i) << " ";
  }
  s << std::endl;
  s << "  Eqs: ";
  for (int i = 0; i < N; i++) {
    s << grid.getEq(i) << " ";
  }
  s << std::endl;
  return s;
}

typedef Grid<1> LineStrip;
typedef Grid<1> Grid1d;
typedef Grid<2> Grid2d;
typedef Grid<3> Grid3d;

arma::sp_mat makeIncompressReg(Grid3d grid);
arma::sp_mat makeIncompressReg(Grid2d grid);

} /* namespace sail */

#endif /* GRID_H_ */
