/*
 *  Created on: 2014-03-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ConcatFunction.h"
#include <assert.h>

namespace sail {


ConcatFunction::ConcatFunction(Function *a, Function *b) {
  Array<std::shared_ptr<Function> > funs(2);
  funs[0] = std::shared_ptr<Function>(a, Function::EmptyDeleter());
  funs[1] = std::shared_ptr<Function>(b, Function::EmptyDeleter());
  initialize(funs);
}

ConcatFunction::ConcatFunction(std::shared_ptr<Function> a,
    std::shared_ptr<Function> b) {
  Array<std::shared_ptr<Function> > funs(2);
  funs[0] = a;
  funs[1] = b;
  initialize(funs);
}

ConcatFunction::ConcatFunction(Array<std::shared_ptr<Function> > funs) {
  initialize(funs);
}

ConcatFunction::ConcatFunction(Array<Function*> funs) {
  initialize(funs.map<std::shared_ptr<Function> >([&] (Function *f)
      {return std::shared_ptr<Function>(f,
    Function::EmptyDeleter());}));
}


namespace {
  int maxOutDims(Array<std::shared_ptr<Function> > funs) {
    int count = funs.size();
    int maxv = 0;
    for (int i = 0; i < count; i++) {
      maxv = std::max(maxv, funs[i]->outDims());
    }
    return maxv;
  }
}

void ConcatFunction::initialize(Array<std::shared_ptr<Function> > funs) {
  _inDims = funs[0]->inDims();
  _Jtemp.create(maxOutDims(funs)*_inDims);
  _functions = funs;
  _outDims = 0;
  int count = funs.size();
  for (int i = 0; i < count; i++) {
    assert(_inDims == funs[i]->inDims());
    _outDims += funs[i]->outDims();
  }
}





void ConcatFunction::eval(double *Xin, double *Fout, double *Jout) {
  Arrayd Fdst(outDims(), Fout);
  MDArray2d Jdst;
  bool outputJ = Jout != nullptr;
  if (outputJ) {
    Jdst = MDArray2d(outDims(), inDims(), Jout);
  }

  int count = _functions.size();
  int offset = 0;
  for (int i = 0; i < count; i++) {
    std::shared_ptr<Function> &f = _functions[i];
    int next = offset + f->outDims();
    f->eval(Xin, Fout + offset, tempPtr(Jout));

    if (outputJ) {
      MDArray2d Jtemp(f->outDims(), f->inDims(), _Jtemp);
      Jtemp.copyToSafe(Jdst.sliceRows(offset, next));
    }
    offset = next;
  }
  assert(offset == _outDims);
}

} /* namespace sail */
