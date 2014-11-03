/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "EdgeDetector1d.h"

namespace sail {

EdgeDetector1d::EdgeDetector1d(double lambda, int minimumEdgeCount) :
    _lambda(lambda), _minimumEdgeCount(minimumEdgeCount) {}

EdgeDetector1d::Result EdgeDetector1d::detect(LineKM sampling, int sampleCount, Arrayd X, Arrayd Y) const {
  Array<LineFitQF> qfs = Array<LineFitQF>::fill(sampleCount, LineFitQF(0));
}


}
