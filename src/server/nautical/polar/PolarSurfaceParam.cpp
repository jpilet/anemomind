/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarSurfaceParam.h>
#include <server/common/Uniform.h>
#include <server/plot/extra.h>


namespace sail {


PolarSurfaceParam::PolarSurfaceParam() :
    _twsLevelCount(0), _alpha(0.1) {}


PolarSurfaceParam::PolarSurfaceParam(PolarCurveParam pcp, Velocity<double> maxTws,
    int twsLevelCount) :
    _twsStep((1.0/twsLevelCount)*maxTws),
    _twsLevelCount(twsLevelCount),
    _alpha(0.1),
    _polarCurveParam(pcp), _maxTws(maxTws) {
    assert(!pcp.empty());
}



Arrayd PolarSurfaceParam::makeInitialParams() const {
  Arrayd params(paramCount());
  LineKM twsAtLevel(0, _twsLevelCount-1,
      _twsStep.knots(), double(_twsLevelCount)*_twsStep.knots());

  for (int i = 0; i < _twsLevelCount; i++) {
    double difToPrev = logline(twsAtLevel(i) - twsAtLevel(i - 1));
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

void PolarSurfaceParam::plot(Arrayd paramsOrVertices,
    GnuplotExtra *dst) {

  Arrayd vertices;
  if (paramsOrVertices.size() == paramCount()) {
    vertices = Arrayd(vertexDim());
    paramToVertices(paramsOrVertices, vertices);
  } else {
    vertices = paramsOrVertices;
  }

  dst->set_style("lines");
  for (int i = 0; i < _twsLevelCount; i++) {
    double z = (i + 1.0)*_twsStep.knots();
    MDArray2d plotData = _polarCurveParam.makePlotData(curveVertices(i, vertices),
      z);
    dst->plot(plotData);
  }
}

} /* namespace mmm */
