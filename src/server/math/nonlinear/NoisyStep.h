/*
 *  Created on: 2014-02-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NOISYSTEP_H_
#define NOISYSTEP_H_

#include <server/math/ADFunction.h>
#include <server/common/Array.h>

namespace sail {

class NoisyStep : public AutoDiffFunction
{
public:
  NoisyStep(Arrayd X, Arrayd Y);

  int inDims();
  int outDims();
  void evalAD(adouble *Xin, adouble *Fout);
private:
  Arrayd _X, _Y;
};

void makeNoisySignalData(int sampleCount, Arrayd &X, Arrayd &Ygt, Arrayd &Ynoisy);

} /* namespace sail */

#endif /* NOISYSTEP_H_ */
