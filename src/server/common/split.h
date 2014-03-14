/*
 *  Created on: 2014-03-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SPLIT_H_
#define SPLIT_H_

#include <server/common/Array.h>

namespace sail {

Arrayb makeRandomSplit(int size);
Array<Arrayb> makeRandomSplits(int numSplits, int size);

} /* namespace sail */

#endif /* SPLIT_H_ */
