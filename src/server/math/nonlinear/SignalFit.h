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

class LevmarSettings;
Arrayd fitLineStripAutoTune(LineStrip strip, Arrayd initRegs, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings);

} /* namespace sail */

#endif /* SIGNALFIT_H_ */
