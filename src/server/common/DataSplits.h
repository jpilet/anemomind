/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DATASPLITS_H_
#define DATASPLITS_H_

#include <server/common/Array.h>
#include <server/common/RandomEngine.h>

namespace sail {

Arrayb makeRandomSplit(int size, RandomEngine::EngineType *e = nullptr);
Array<Arrayb> makeRandomSplits(int numSplits, int size, RandomEngine::EngineType *e = nullptr);
Arrayb makeSlidedSplit(int count, RandomEngine::EngineType *e = nullptr);

}

#endif /* DATASPLITS_H_ */
