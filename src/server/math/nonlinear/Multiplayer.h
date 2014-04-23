/*
 *  Created on: Apr 23, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MULTIPLAYER_H_
#define MULTIPLAYER_H_

#include <functional>
#include <server/common/Array.h>
#include <server/common/Function.h>
#include <memory>

namespace sail {

class StepMinimizer;
Arrayd optimizeMultiplayer(const StepMinimizer &minimizer,
    Array<std::shared_ptr<Function> > objfs, Arrayd X,
    Arrayd initStepSizes = Arrayd());

} /* namespace sail */

#endif /* MULTIPLAYER_H_ */
