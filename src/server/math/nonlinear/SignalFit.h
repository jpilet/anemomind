/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SIGNALFIT_H_
#define SIGNALFIT_H_

#include <server/math/Grid.h>
#include <server/math/BandMat.h>
#include <memory>

namespace sail {

Arrayi makeDefaultRegOrders(int n);

class SignalFitResults {
 public:
  SignalFitResults(Arrayd rw, Arrayd v) : regWeights(rw), vertices(v) {}

  Arrayd regWeights, vertices;
};

class LevmarSettings;

// Orders and reg weights
SignalFitResults fitLineStripAutoTune(LineStrip strip, Arrayd initRegs, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings);

// Default orders and reg weights
SignalFitResults fitLineStripAutoTune(LineStrip strip, Arrayi orders, Arrayd initRegs, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings);

// Orders and default reg weights
SignalFitResults fitLineStripAutoTune(LineStrip strip, Arrayi orders, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings);


} /* namespace sail */

#endif /* SIGNALFIT_H_ */
