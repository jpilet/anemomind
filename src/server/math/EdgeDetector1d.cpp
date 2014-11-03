/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "EdgeDetector1d.h"
#include <cassert>

namespace sail {

EdgeDetector1d::EdgeDetector1d(double lambda, int minimumEdgeCount) :
    _lambda(lambda), _minimumEdgeCount(minimumEdgeCount) {}

namespace {
  Array<LineFitQF> buildQfs(LineKM sampling, int sampleCount, Arrayd X, Arrayd Y) {
    Array<LineFitQF> qfs = Array<LineFitQF>::fill(sampleCount, LineFitQF(0));
    int count = X.size();
    assert(count == Y.size());
    for (int i = 0; i < count; i++) {
      int index = int(floor(sampling.inv(X[i])));
      qfs[index] += LineFitQF::fitLine(X[i], Y[i]);
    }
    return qfs;
  }

}

EdgeDetector1d::Result EdgeDetector1d::detect(LineKM sampling, int sampleCount, Arrayd X, Arrayd Y) const {
  Array<LineFitQF> qfs = buildQfs(sampling, sampleCount, X, Y);
  Integral1d<LineFitQF> integral(qfs);
}


}
