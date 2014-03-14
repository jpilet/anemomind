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

Arrayd fitLineStrip(LineStrip strip,
    Arrayi orders, Arrayd regWeights,
    Arrayd X, Arrayd Y);

Arrayi makeDefaultRegOrders(int n);

class SignalFitResults {
 public:
  SignalFitResults(Arrayi o, Arrayd rw, Arrayd v) :
    orders(o), regWeights(rw), vertices(v) {}

  Arrayi orders;
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
