/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CurveParam.h"
#include <server/math/BandMat.h>
#include <armadillo>

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
        _coefs[cind];
      }
      return 0;
    }

    //BandMat<double> makeBandMat() const;
    arma::mat makeRegMat() const;
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
      _coefs = BandMatInternal::makeNextCoefs(_coefs);
    }
    assert(_coefs.size() == 1 + regDeg);
  }

  arma::mat RegMat::makeRegMat() const {
    arma::mat A = arma::zeros(rows(), cols());
    for (int i = 0; i < rows(); i++) {

    }
    return A;
  }
}

MDArray2d parameterizeOpenCurve(int vertexCount, Arrayi ctrlInds, int regDeg) {

}

} /* namespace mmm */
