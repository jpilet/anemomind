/*
 *  Created on: 14 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MEANSTABILIZER_H_
#define MEANSTABILIZER_H_

#include <server/common/MDArray.h>
#include <server/common/Function.h>

namespace sail {

/*
 * A function to stabilize the convergence of the parameters
 * in a Levenberg-Marquardt algorithm.
 */
class MeanStabilizer : public Function {
 public:
  MeanStabilizer(MDArray2d Jref, double s);
  MeanStabilizer(Function &objf, Arrayd X, double s);
  void eval(double *Xin, double *Fout, double *Jout);
  int inDims() {return _dims;}
  int outDims() {return _dims;}
 private:
  void initialize(int dims, double weight);
  void initialize(MDArray2d Jref, double s);
  int _dims;
  double _weight;
};

} /* namespace sail */

#endif /* MEANSTABILIZER_H_ */
