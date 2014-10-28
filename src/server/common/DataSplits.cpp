/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "DataSplits.h"
#include <server/common/Uniform.h>
#include <algorithm>

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

namespace {

  Arrayb initializeMarked(int len) {
    Arrayb marked(len);
    int middle = marked.size()/2;
    marked.sliceTo(middle).setTo(false);
    marked.sliceFrom(middle).setTo(true);
    return marked;
  }

  Arrayb rotate(Arrayb src) {
    int len = src.size();
    Uniform rng(len);
    int offset = rng.gen();
    Arrayb dst(len);
    for (int i = 0; i < len; i++) {
      dst[i] = src[(i + offset) % len];
    }
    return dst;
  }
}

Arrayb makeChunkSplit(int length, double probNext) {
  assert(0 < probNext);
  assert(probNext < 1.0);
  Arrayb marked = initializeMarked(length);
  int middle = length/2;
  Uniform rng(0, 1);
  while (true) {
    marked = rotate(marked);
    if (rng.gen() < probNext) {
      std::reverse(marked.begin(), marked.ptr(middle));
    } else {
      break;
    }
  }
  return marked;
}

Array<Arrayb> makeChunkSplits(int count, int length, double probNext) {
  return Array<Arrayb>::fill(count, [=](int indexNotUsed) {
    return makeChunkSplit(length, probNext);
  });
}




}
