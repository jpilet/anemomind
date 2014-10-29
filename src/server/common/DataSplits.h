/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DATASPLITS_H_
#define DATASPLITS_H_

#include <server/common/Array.h>
#include <random>

namespace sail {

Arrayb makeRandomSplit(int size, std::default_random_engine &e);
Array<Arrayb> makeRandomSplits(int numSplits, int size, std::default_random_engine &e);
Arrayb makeSlidedSplit(int count, std::default_random_engine &e);

}

#endif /* DATASPLITS_H_ */
