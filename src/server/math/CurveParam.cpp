/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CurveParam.h"
#include <server/math/BandMat.h>
#include <armadillo>
#include <server/math/armaadolc.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>

namespace sail {

namespace {
  class RegMat {
   public:
    RegMat(int vertexCount, int regDeg, bool open);

    int rows() const {
      return _vertexCount - (_open?  _regDeg : 0);
    }

    int cols() const {
      return _vertexCount;
    }

    double get(int i, int j) const {
      int cind = (j % cols()) - i;
      if (0 <= cind && cind < _coefs.size()) {
        return _coefs[cind];
      }
      return 0;
    }

    //BandMat<double> makeBandMat() const;
    MDArray2d makeRegMat() const;
   private:
    Arrayd _coefs;
    int _vertexCount, _regDeg;
    bool _open;
  };

  RegMat::RegMat(int vertexCount, int regDeg, bool open) :
    _vertexCount(vertexCount),
    _regDeg(regDeg),
    _open(open) {
    _coefs = Arrayd::args(1.0);
    for (int i = 0; i < regDeg; i++) {
      _coefs = makeNextRegCoefs(_coefs);
    }
    assert(_coefs.size() == 1 + regDeg);
  }

  MDArray2d RegMat::makeRegMat() const {
    MDArray2d A(rows(), cols());
    A.setAll(0);
    for (int i = 0; i < rows(); i++) {
      for (int j = 0; j < cols(); j++) {
        A(i, j) = get(i, j);
      }
    }
    return A;
  }

  void splitA(MDArray2d A, arma::mat *B, arma::mat *C, Arrayb mask, int ctrlCount) {
    int rows = A.rows();
    *B = arma::mat(rows, A.cols() - ctrlCount);
    *C = arma::mat(rows, ctrlCount);
    for (int i = 0; i < rows; i++) {
      int b = 0;
      int c = 0;
      for (int j = 0; j < A.cols(); j++) {
        if (mask[j]) {
          (*C)(i, c) = A(i, j);
          c++;
        } else {
          (*B)(i, b) = -A(i, j);
          b++;
        }
      }
      assert(b + c == A.cols());
    }
  }

  MDArray2d buildFullP(arma::mat P0mat, Arrayb mask) {
    MDArray2d P0 = toMDArray(P0mat);
    int rows = mask.size();
    int cols = P0mat.n_cols;
    MDArray2d P(rows, cols);
    P.setAll(0);
    int p0counter = 0;
    int ctrlCounter = 0;
    for (int i = 0; i < rows; i++) {
      if (mask[i]) {
        P(i, ctrlCounter) = 1.0;
        ctrlCounter++;
      } else {
        P0.sliceRow(p0counter).copyToSafe(P.sliceRow(i));
        p0counter++;
      }
    }
    return P;
  }
}

MDArray2d parameterizeCurve(int vertexCount, Arrayi ctrlInds, int regDeg, bool open) {

  // TODO: Implement this function using banded matrices
  // Currently:          O(n^3) to compute
  // With banded matrix: O(n) to compute

  RegMat rm(vertexCount, regDeg, open);
  MDArray2d A = rm.makeRegMat();
  Arrayb mask = Arrayb::fill(vertexCount, false);
  for (auto index: ctrlInds) {
    mask[index] = true;
  }
  arma::mat B, C;
  splitA(A, &B, &C, mask, ctrlInds.size());
  arma::mat P0 = solve(B, C);
  return buildFullP(P0, mask);
}

} /* namespace mmm */
