/*
 *  Created on: Apr 23, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MULTIPLAYER_H_
#define MULTIPLAYER_H_

#include <functional>
#include <server/common/Array.h>

namespace sail {

class StepMinimizer;
Arrayd optimizeMultiplayer(StepMinimizer &minimizer,
    Array<std::function<double(double)> > objfs, Arrayd X,
    Arrayd initStepSizes = Arrayd());

} /* namespace sail */

#endif /* MULTIPLAYER_H_ */
