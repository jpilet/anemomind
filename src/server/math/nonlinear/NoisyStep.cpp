/*
 *  Created on: 11 févr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NoisyStep.h"
#include <vector>
#include <algorithm>
#include <server/common/Uniform.h>

namespace sail {

void makeNoisySignalData(int sampleCount, Arrayd &X, Arrayd &Ygt, Arrayd &Ynoisy) {
  Uniform rng(-1.0, 1.0);
  X.create(sampleCount);
  Ygt.create(sampleCount);
  Ynoisy.create(sampleCount);
  Uniform noise(-0.2, 0.2);

  std::vector<double> Xsorted(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    Xsorted[i] = rng.gen();
  }
  std::sort(Xsorted.begin(), Xsorted.end());


  for (int i = 0; i < sampleCount; i++) {
    X[i] = Xsorted[i];
    Ygt[i] = (X[i] < 0? -1.0 : 1.0);
    Ynoisy[i] = Ygt[i] + noise.gen();
  }
}




NoisyStep::NoisyStep(Arrayd X, Arrayd Y) : _X(X), _Y(Y) {
  assert(X.size() == Y.size());
}

int NoisyStep::inDims() {
  return 1;
}

int NoisyStep::outDims() {
  return _X.size();
}

void NoisyStep::evalAD(adouble *Xin, adouble *Fout) {
  adouble &x = Xin[0];
  for (int i = 0; i < _X.size(); i++) {
    Fout[i] = _Y[i] + x*pow(_X[i], 3);
  }
}


} /* namespace sail */
