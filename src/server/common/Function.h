/*
 * Function.h
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#ifndef FUNCTION_H_
#define FUNCTION_H_

namespace sail {

class Function {
 public:
  // Provide an instance of this object as extra parameter when creating
  // a shared_ptr referring to an object on the stack.
  class EmptyDeleter {
   public:
    void operator() (Function *x) {/*Do nothing: don't deallocate the object*/}
  };

  Function() {}

  // TO BE OVERRIDDEN:
  virtual int inDims() = 0;
  virtual int outDims() = 0;
  virtual void eval(double *Xin, double *Fout, double *Jout = nullptr) = 0;

  // This function outputs the numeric Jacobian to JNumOut through
  // numerical differentiation of 'eval'. The output matrix is stored
  // in column major format, that is an element at (i, j) has index
  // i + j*outDims().
  void evalNumericJacobian(double *Xin, double *JNumOut, double h = 1.0e-6);

  double calcSquaredNorm(double *X, double *Fscratch);
  virtual ~Function() {}
};

} /* namespace sail */

#endif /* FUNCTION_H_ */
