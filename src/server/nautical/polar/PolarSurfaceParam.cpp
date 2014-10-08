/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarSurfaceParam.h>
#include <server/common/Uniform.h>
#include <server/plot/extra.h>
#include <server/math/CurveParam.h>


namespace sail {


PolarSurfaceParam::PolarSurfaceParam() :
    _twsLevelCount(0), _ctrlCount(0) {}


namespace {
  Arrayi makeSpacedCtrlInds(int vertexCount, int ctrlCount) {
    LineKM vindex(0, ctrlCount-1, 0, vertexCount-1);
    Arrayi inds(ctrlCount);
    for (int i = 0; i < ctrlCount; i++) {
      inds[i] = int(round(vindex(i)));
    }
    return inds;
  }
}

PolarSurfaceParam::PolarSurfaceParam(PolarCurveParam pcp, Velocity<double> maxTws,
    int twsLevelCount, int ctrlCount) :
  _polarCurveParam(pcp),
  _twsStep((1.0/twsLevelCount)*maxTws),
  _maxTws(maxTws),
  _twsLevelCount(twsLevelCount),
  _ctrlCount(ctrlCount == -1? twsLevelCount : ctrlCount) {
    assert(!pcp.empty());
    _ctrlInds = makeSpacedCtrlInds(_twsLevelCount, _ctrlCount);

    if (ctrlCount != -1) {
      _P = parameterizeCurve(_twsLevelCount, _ctrlInds,
          2, true);
    }
}



Arrayd PolarSurfaceParam::makeInitialParams() const {
  Arrayd params(paramCount());
  LineKM twsAtLevel(0, _twsLevelCount-1,
      _twsStep.knots(), 0.3*double(_twsLevelCount)*_twsStep.knots());
  if (withCtrl()) {
    curveParams(0, params).setTo(logline(twsAtLevel(0)));
    for (int i = 1; i < _ctrlCount; i++) {
      double difToPrev = logline(twsAtLevel(_ctrlInds[i]) - twsAtLevel(_ctrlInds[i - 1]));
      curveParams(i, params).setTo(difToPrev);
    }
  } else {
    for (int i = 0; i < _twsLevelCount; i++) {
      double difToPrev = logline(twsAtLevel(i) - twsAtLevel(i - 1));
      curveParams(i, params).setTo(difToPrev);
    }
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


void PolarSurfaceParam::toSurfaceCoord(Velocity<double> tws, Angle<double> twa, double *outXY) const {
  assert(0.0 <= tws.knots()); assert(tws <= _maxTws);
  outXY[0] = twa.positiveMinAngle().degrees()/360.0;
  outXY[1] = tws/_maxTws;
}

Velocity<double> PolarSurfaceParam::targetSpeed(Arrayd vertices, double surfaceCoord2[2]) const {
    // Bilinear interpolation

  Velocity<double> xy[2] = {Velocity<double>::knots(0), Velocity<double>::knots(0)};
  double curvep = surfaceCoord2[0];
  double curveIndex = surfaceCoord2[1]*_twsLevelCount - 1;
  int lower = std::min(int(floor(curveIndex)), _twsLevelCount-1);
  int upper = lower + 1;
  double lambda = curveIndex - lower;
  double lowerWeight = 1.0 - lambda;
  double upperWeight = lambda;
  addWeightedCurveVertex(vertices, lower,
      lowerWeight, curvep, xy);
  addWeightedCurveVertex(vertices, upper,
      upperWeight, curvep, xy);

  // Return the distance to the origin
  double sqnorm = sqr(xy[0].knots()) + sqr(xy[1].knots());
  return Velocity<double>::knots(sqrt(sqnorm));
}

  Velocity<double> PolarSurfaceParam::targetSpeed(Arrayd vertices,
    Velocity<double> tws, Angle<double> twa) const {

  double surfaceCoord2[2];
  toSurfaceCoord(tws, twa, surfaceCoord2);
  return targetSpeed(vertices, surfaceCoord2);
}


Arrayd PolarSurfaceParam::toVertices(Arrayd paramsOrVertices) const {
  if (paramsOrVertices.size() == paramCount()) {
    Arrayd vertices(vertexDim());
    paramToVertices(paramsOrVertices, vertices);
    return vertices;
  } else {
    return paramsOrVertices;
  }
}

void PolarSurfaceParam::plot(Arrayd paramsOrVertices,
    GnuplotExtra *dst) const {

  Arrayd vertices = toVertices(paramsOrVertices);

  dst->set_style("lines");
  dst->set_pointsize(2);
  for (int i = 0; i < _twsLevelCount; i++) {
    double z = (i + 1.0)*_twsStep.knots();
    MDArray2d plotData = _polarCurveParam.makePlotData(curveVertices(i, vertices),
      z);
    dst->plot(plotData);
  }
}

MDArray2d PolarSurfaceParam::makeVertexData(Arrayd paramsOrVertices) const {
  Arrayd v = toVertices(paramsOrVertices);
  MDArray2d data(_polarCurveParam.vertexCount(), 3*_twsLevelCount);
  for (int i = 0; i < _twsLevelCount; i++) {
    double z = (i + 1.0)*_twsStep.knots();
    int offs = 3*i;
    _polarCurveParam.makePlotData(curveVertices(i, v), z)
        .copyToSafe(data.sliceCols(offs, offs + 3));
  }
  return data;
}

void PolarSurfaceParam::plot(Arrayd paramsOrVertices) const {
  GnuplotExtra p;
  plot(paramsOrVertices, &p);
  p.show();
}

} /* namespace mmm */
