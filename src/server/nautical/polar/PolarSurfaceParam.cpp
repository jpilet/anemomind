/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarSurfaceParam.h>
#include <server/common/Uniform.h>

namespace sail {

PolarSurfaceParam::PolarSurfaceParam() :
    _twsLevelCount(0) {}


PolarSurfaceParam::PolarSurfaceParam(PolarCurveParam pcp, Velocity<double> maxTws,
    int twsLevelCount) :
    _twsStep((1.0/twsLevelCount)*maxTws),
    _twsLevelCount(twsLevelCount),
    _polarCurveParam(pcp), _maxTws(maxTws) {
    assert(!pcp.empty());
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

Array<Vectorize<double, 2> > PolarSurfaceParam::generateSurfacePoints(int count) {
  Uniform rng(0, 1);
  Array<Vectorize<double, 2> > dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = Vectorize<double, 2>{rng.gen(),
        sqrt(rng.gen())}; // <-- Take the square root here,
                          // because we want the sampling density
                          // to be equally dense for low as
                          // well as for high wind speeds.
  }
  return dst;
}

} /* namespace mmm */
