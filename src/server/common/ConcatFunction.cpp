/*
 *  Created on: 2014-03-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ConcatFunction.h"
#include <assert.h>

namespace sail {


ConcatFunction::ConcatFunction(Function &a, Function &b) : _a(a), _b(b) {
  assert(_a.inDims() == _b.inDims());
  _Jtemp.create(std::max(_a.outDims(), _b.outDims())*inDims());
}

MDArray2d ConcatFunction::Atemp() {
  return MDArray2d(_a.outDims(), _a.inDims(), _Jtemp.ptr());
}
MDArray2d ConcatFunction::Btemp() {
  return MDArray2d(_b.outDims(), _b.inDims(), _Jtemp.ptr());
}


void ConcatFunction::eval(double *Xin, double *Fout, double *Jout) {
  Arrayd Fdst(outDims(), Fout);
  MDArray2d Jdst;
  bool outputJ = Jout != nullptr;
  if (outputJ) {
    Jdst = MDArray2d(outDims(), inDims(), Jout);
  }

  _a.eval(Xin, Fout, tempPtr(Jout));
  if (outputJ) {
    Atemp().copyToSafe(Jdst.sliceRowsTo(_a.outDims()));
  }

  _b.eval(Xin, Fout + _a.outDims(), tempPtr(Jout));
  if (outputJ) {
    Btemp().copyToSafe(Jdst.sliceRowsFrom(_a.outDims()));
  }
}

} /* namespace sail */
