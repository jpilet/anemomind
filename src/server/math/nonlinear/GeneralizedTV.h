/*
 *  Created on: 2014-10-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GENARALIZEDTV_H_
#define GENARALIZEDTV_H_

#include <server/math/UniformSamples.h>

namespace sail {

class GeneralizedTV {
 public:
  GeneralizedTV(int iters = 30, double minv = 1.0e-6,
      double gaussElimTol = 1.0e-6);

  UniformSamples filter(UniformSamples initialSignal,
                  Arrayd X, Arrayd Y,
                  int order,
                  double regularization) const;

  UniformSamples filter(Arrayd X, Arrayd Y, double samplingDensity,
                    int order,
                    double regularization) const;

  // The values of the signal Y are assumed to be
  // have a corresponding X vector ranging from 0 to Y.size()-1.
  // A signal is returned that can be evaluated at those locations.
  UniformSamples filter(Arrayd Y, int order, double regularization) const;

  static Arrayd makeDefaultX(int size);
  static UniformSamples makeInitialSignal(Arrayd Y);
  static UniformSamples makeInitialSignal(Arrayd X, Arrayd Y, double sampleSpacing);
 private:
  // Settings
  int _iters;
  double _minv, _gaussElimTol;

  UniformSamples step(UniformSamples signal,
      Arrayd X, Arrayd Y, Arrayd coefs, double regularization) const;
};



}

#endif /* GENARALIZEDTV_H_ */
