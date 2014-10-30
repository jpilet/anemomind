/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/DataSplits.h>
#include <algorithm>

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

namespace {
  Arrayb initializeMarked(int len) {
    Arrayb marked(len);
    int middle = marked.size()/2;
    marked.sliceTo(middle).setTo(false);
    marked.sliceFrom(middle).setTo(true);
    return marked;
  }

  void rotateRandom(Arrayb src, std::default_random_engine &e) {
    int len = src.size();
    std::uniform_int_distribution<int> distrib(0, len-1);
    int offset = distrib(e);
    std::rotate(src.begin(), src.begin() + offset, src.end());
  }
}

Arrayb makeChunkSplit(int length, std::default_random_engine &e, double probNext) {
  assert(0 < probNext);
  assert(probNext < 1.0);
  Arrayb marked = initializeMarked(length);
  int middle = length/2;
  std::uniform_real_distribution<double> next(0, 1);
  do {
    std::reverse(marked.begin(), marked.ptr(middle));
    rotateRandom(marked, e);
  } while (next(e) < probNext);
  return marked;
}

Array<Arrayb> makeChunkSplits(int count, int length,
    std::default_random_engine &e, double probNext) {
  return Array<Arrayb>::fill(count, [&](int indexNotUsed) {
    return makeChunkSplit(length, e, probNext);
  });
}




}
