/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarCurveParam.h>
#include <server/math/CurveParam.h>
#include <server/common/LineKM.h>
#include <cassert>

namespace sail {

PolarCurveParam::PolarCurveParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored) :
    _segsPerCtrlSpan(segsPerCtrlSpan), _ctrlCount(ctrlCount), _mirrored(mirrored) {
    if (mirrored) {
      _paramCount = div1(ctrlCount, 2);
    } else {
      _paramCount = ctrlCount;
    }

    _P = parameterizeCurve(vertexCount(), makeCtrlInds(), 2, true);
}

Angle<double> PolarCurveParam::ctrlAngle(int ctrlIndex) const {
  assert(0 <= ctrlIndex);
  assert(ctrlIndex < _ctrlCount);
  LineKM line(-1, _ctrlCount, 0.0, 360.0);
  return Angle<double>::degrees(line(ctrlIndex));
}

Arrayi PolarCurveParam::makeCtrlInds() const {
  Arrayi inds(_ctrlCount);
  for (int i = 0; i < _ctrlCount; i++) {
    inds[i] = ctrlToVertexIndex(i);
  }
  return inds;
}

int PolarCurveParam::ctrlToParamIndex(int ctrlIndex) const {
  assert(0 <= ctrlIndex);
  assert(ctrlIndex < _ctrlCount);
  if (ctrlIndex < _paramCount) {
    return ctrlIndex;
  }
  int lastParamIndex = _paramCount - 1;
  int overflow = ctrlIndex - lastParamIndex;
  return lastParamIndex - overflow;
}

} /* namespace mmm */
