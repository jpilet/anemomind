/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DATASPLITS_H_
#define DATASPLITS_H_

#include <server/common/Array.h>

namespace sail {

Arrayb makeRandomSplit(int size);
Array<Arrayb> makeRandomSplits(int numSplits, int size);
Arrayb makeSlidedSplit(int count);
Arrayb makeChunkSplit(int length, double probNext = 0.7);
Array<Arrayb> makeChunkSplits(int count, int length, double probNext = 0.7);

}

#endif /* DATASPLITS_H_ */
