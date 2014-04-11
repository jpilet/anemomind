/*
<<<<<<< HEAD
 *  Created on: 14 mars 2014
=======
 *  Created on: 2014-03-14
>>>>>>> master
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CONCATFUNCTION_H_
#define CONCATFUNCTION_H_

#include <server/common/Function.h>
#include <server/common/MDArray.h>
#include <memory>

namespace sail {

class ConcatFunction : public Function {
 public:
  // Concatenate functions pointed to by shared pointers
  ConcatFunction(std::shared_ptr<Function> a, std::shared_ptr<Function> b);
  ConcatFunction(Array<std::shared_ptr<Function> > funs);

  int inDims() {return _inDims;}
  int outDims() {return _outDims;}
  void eval(double *Xin, double *Fout, double *Jout);
 private:
  void initialize(Array<std::shared_ptr<Function> > funs);
  int _outDims, _inDims;
  Array<std::shared_ptr<Function> > _functions;

  Arrayd _Jtemp;
  double *tempPtr(double *Jout) {return (Jout == nullptr? nullptr : _Jtemp.ptr());}
};

} /* namespace sail */

#endif /* CONCATFUNCTION_H_ */
