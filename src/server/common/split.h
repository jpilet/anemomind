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
Arrayb makeFoldSplit(int size, int numberOfPieces, int index, bool train = true);
Arrayb makeRandomlySlided2Fold(int size);

} /* namespace sail */

#endif /* SPLIT_H_ */
