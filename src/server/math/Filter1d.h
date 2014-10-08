/*
 *  Created on: 2014-10-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FILTER1D_H_
#define FILTER1D_H_

#include <server/common/Array.h>
#include <server/math/Grid.h>
#include <server/math/nonlinear/LevmarSettings.h>

namespace sail {

Arrayd filter1d(LineStrip strip, Arrayd X, Arrayd Y,
    Arrayi regOrders, Arrayd regWeights,
    Array<Arrayb> crossValidationSplits,
    LevmarSettings s = LevmarSettings());


Arrayd filter1d(Arrayd X,
    Arrayi orders, Arrayd weights,
    Array<Arrayb> crossValidationSplits,
    LevmarSettings s = LevmarSettings());

/*
 * Filters a signal with default settings.
 */
Arrayd filter1dEasy(Arrayd X,
    int order = 1, double initWeight = 10.0,
    Array<Arrayb> crossValidationSplits = Array<Arrayb>(8),
    LevmarSettings s = LevmarSettings());

}

#endif /* FILTER1D_H_ */
