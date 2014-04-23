/*
 *  Created on: 2014-04-23
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Solve a multiplayer game.
 */

#ifndef MULTIPLAYER_H_
#define MULTIPLAYER_H_

#include <functional>
#include <server/common/Array.h>
#include <server/common/Function.h>
#include <memory>

namespace sail {


/*
 * Minimize each scalar-valued function in objfs w.r.t. a
 * corresponding parameter in X.
 */
class StepMinimizer;
Arrayd optimizeMultiplayer(const StepMinimizer &minimizer,
    Array<std::shared_ptr<Function> > objfs, Arrayd X,
    Arrayd initStepSizes = Arrayd());

} /* namespace sail */

#endif /* MULTIPLAYER_H_ */
