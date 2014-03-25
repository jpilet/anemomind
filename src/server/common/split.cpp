/*
 *  Created on: 14 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "split.h"
#include <server/common/Uniform.h>
#include <server/common/LineKM.h>

namespace sail {

Arrayb makeRandomSplit(int count) {
  assert(count >= 2);
  Uniform rng(0.0, 1.0);
  Arrayb split(count);
  int trueCount = 0;
  for (int i = 0; i < count; i++) {
    bool incl = rng.gen() > 0.5;
    split[i] = incl;
    trueCount += (incl? 1 : 0);
  }
  if (trueCount == 0 || trueCount == count) { // <-- If this condition is satisfied, the
                                              //     resulting split could result in sin-
                                              //     gular matrices. Therefore, find a
                                              //     new split.

    return makeRandomSplit(count); // <-- Infinite recursion will not happen: The probability that
                                   //     we will have found a valid split after N tries tends
                                   //     to 1 as N tends towards infinity. However, we require count >= 2.
  }
  return split;
}

Array<Arrayb> makeRandomSplits(int numSplits, int size) {
  Array<Arrayb> dst(numSplits);
  for (int i = 0; i < numSplits; i++) {
    dst[i] = makeRandomSplit(size);
  }
  return dst;
}

Arrayb makeFoldSplit(int size, int numberOfPieces, int index, bool train) {
  LineKM indexGen(0, numberOfPieces, 0, size-1 + 0.001);
  Arrayb incl(size);
  incl.setTo(train);
  incl.slice(int(indexGen(index)), int(indexGen(index+1))).setTo(!train);
  return incl;
}

Arrayb makeRandomlySlidedFold(int size) {
  Uniform g(0.0, 1.0);
  bool k = g.gen() > 0.5;
  Arrayb split(size);
  split.setTo(k);

  Uniform gen(size);
  int a = gen.genInt();
  int b = (a + size/2) % size;
  int minv = std::min(a, b);
  int maxv = std::max(a, b);

  split.slice(minv, maxv).setTo(!k);
  return split;
}

Array<Arrayb> makeRandomlySlidedFolds(int count, int size, bool flip) {
  int step = (flip? 2 : 1);
  Array<Arrayb> splits(step*count);
  for (int i = 0; i < count; i++) {
    int offs = step*i;
    splits[offs + 0] = makeRandomlySlidedFold(size);
    if (flip) {
      splits[offs + 1] = neg(splits[offs + 0]);
    }
  }
  return splits;
}

} /* namespace sail */
