/*
 *  Created on: 2014-03-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CONCATFUNCTION_H_
#define CONCATFUNCTION_H_

#include <server/common/Function.h>
#include <server/common/MDArray.h>

namespace sail {

class ConcatFunction : public Function {
 public:
  ConcatFunction(Function &a, Function &b);
  int inDims() {return _a.inDims();}
  int outDims() {return _a.outDims() + _b.outDims();}
  void eval(double *Xin, double *Fout, double *Jout);
 private:
  Function &_a, &_b;
  Arrayd _Jtemp;

  MDArray2d Atemp();
  MDArray2d Btemp();
  double *tempPtr(double *Jout) {return (Jout == nullptr? nullptr : _Jtemp.ptr());}
};

} /* namespace sail */

#endif /* CONCATFUNCTION_H_ */
