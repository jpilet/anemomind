/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "DataSplits.h"

namespace sail {

Arrayb makeRandomSplit(int count, std::default_random_engine &e) {
  assert(count >= 2);
  std::uniform_real_distribution<double> distrib(0, 1);
  Arrayb split(count);
  int trueCount = 0;
  for (int i = 0; i < count; i++) {
    bool incl = distrib(e) > 0.5;
    split[i] = incl;
    trueCount += (incl? 1 : 0);
  }
  if (trueCount == 0 || trueCount == count) { // <-- If this condition is satisfied, the
                                              //     resulting split could result in sin-
                                              //     gular matrices. Therefore, find a
                                              //     new split.

    return makeRandomSplit(count, e); // <-- Infinite recursion will not happen: The probability that
                                   //     we will have found a valid split after N tries tends
                                   //     to 1 as N tends towards infinity. However, we require count >= 2.
  }
  return split;
}

Array<Arrayb> makeRandomSplits(int numSplits, int size, std::default_random_engine &e) {
  Array<Arrayb> dst(numSplits);
  for (int i = 0; i < numSplits; i++) {
    dst[i] = makeRandomSplit(size, e);
  }
  return dst;
}

Arrayb makeSlidedSplit(int count, std::default_random_engine &e) {
  std::uniform_int_distribution<int> distrib(0, count-1);
  int offset = distrib(e);
  int middle = count/2;
  Arrayb dst(count);
  for (int i = 0; i < count; i++) {
    dst[(offset + i) % count] = i < middle;
  }
  return dst;
}


}
