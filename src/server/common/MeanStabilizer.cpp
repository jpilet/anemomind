/*
 *  Created on: 14 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "MeanStabilizer.h"
#include <server/common/math.h>

namespace sail {

namespace {
  double calcMaxSqrtJtJDiagElem(MDArray2d J) {
    double me = 0.0;
    for (int i = 0; i < J.cols(); i++) {
      double e = norm(J.rows(), J.sliceCol(i).ptr());
      if (e > me) {
        me = e;
      }
    }
    return me;
  }
}

void MeanStabilizer::initialize(MDArray2d Jref, double s) {
  double me = calcMaxSqrtJtJDiagElem(Jref);
  initialize(Jref.cols(), s*me);
}

MeanStabilizer::MeanStabilizer(Function &objf, Arrayd X, double s) {
  MDArray2d J(objf.outDims(), objf.inDims());
  Arrayd F(objf.outDims());
  objf.eval(X.ptr(), F.ptr(), J.ptr());
  initialize(J, s);
}

MeanStabilizer::MeanStabilizer(MDArray2d Jref, double s) {
  initialize(Jref, s);
}

void MeanStabilizer::eval(double *Xin, double *Fout, double *Jout) {
  bool outputJ = Jout != nullptr;
  MDArray2d Jdst;
  if (outputJ) {
    Jdst = MDArray2d(_dims, _dims, Jout);
  }

  for (int i = 0; i < _dims; i++) {
    Fout[i] = 0.0;
  }

  double mean = 1.0/_dims;
  if (outputJ) {
    for (int i = 0; i < _dims; i++) {
      for (int j = 0; j < _dims; j++) {
        double x = (i == j? 1.0 : 0.0);
        Jdst(i, j) = _weight*(x - mean);
      }
    }
  }
}

void MeanStabilizer::initialize(int dims, double weight) {
  _dims = dims;
  _weight = weight;
}


} /* namespace sail */
