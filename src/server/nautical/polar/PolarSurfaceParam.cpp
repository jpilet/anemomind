/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarSurfaceParam.h>

namespace sail {

PolarSurfaceParam::PolarSurfaceParam() :
    _twsLevelCount(0) {}


PolarSurfaceParam::PolarSurfaceParam(PolarCurveParam pcp, Velocity<double> maxTws,
    int twsLevelCount) :
    _twsStep((1.0 / twsLevelCount) * maxTws), _twsLevelCount(twsLevelCount), _polarCurveParam(
        pcp) {
}



Arrayd PolarSurfaceParam::makeInitialParams() const {
  Arrayd params(vertexDim());
  LineKM twsAtLevel(0, _twsLevelCount-1,
      _twsStep.knots(), double(_twsLevelCount)*_twsStep.knots());

  for (int i = 0; i < _twsLevelCount; i++) {
    double difToPrev = log(twsAtLevel(i) - twsAtLevel(i - 1));
    curveParams(i, params).setTo(difToPrev);
  }
  return params;
}

} /* namespace mmm */
