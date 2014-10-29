/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "DataSplits.h"
#include <server/common/Uniform.h>
#include <algorithm>

namespace sail {

Arrayb makeRandomSplit(int count, RandomEngine::EngineType *e) {
  assert(count >= 2);
  RandomEngine::EngineType &engine = RandomEngine::get(e);
  std::uniform_real_distribution<double> distrib(0, 1);
  Arrayb split(count);
  int trueCount = 0;
  for (int i = 0; i < count; i++) {
    bool incl = distrib(engine) > 0.5;
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

Array<Arrayb> makeRandomSplits(int numSplits, int size, RandomEngine::EngineType *e) {
  Array<Arrayb> dst(numSplits);
  for (int i = 0; i < numSplits; i++) {
    dst[i] = makeRandomSplit(size, e);
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

  void rotateRandom(Arrayb src, RandomEngine::EngineType *e) {
    int len = src.size();
    //Uniform rng(len);
    std::uniform_int_distribution<int> distrib(0, len-1);
    int offset = distrib(RandomEngine::get(e));
    std::rotate(src.begin(), src.begin() + offset, src.end());
  }
}

Arrayb makeChunkSplit(int length, double probNext, RandomEngine::EngineType *e) {
  assert(0 < probNext);
  assert(probNext < 1.0);
  Arrayb marked = initializeMarked(length);
  int middle = length/2;
  RandomEngine::EngineType &eng = RandomEngine::get(e);
  std::uniform_real_distribution<double> next(0, 1);
  do {
    std::reverse(marked.begin(), marked.ptr(middle));
    rotateRandom(marked, e);
  } while (next(eng) < probNext);
  return marked;
}

Array<Arrayb> makeChunkSplits(int count, int length, double probNext,
    RandomEngine::EngineType *e) {
  return Array<Arrayb>::fill(count, [=](int indexNotUsed) {
    return makeChunkSplit(length, probNext, e);
  });
}




}
