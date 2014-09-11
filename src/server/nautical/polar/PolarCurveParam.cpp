/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarCurveParam.h>
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
}

Angle<double> PolarCurveParam::ctrlAngle(int ctrlIndex) const {
  assert(0 <= ctrlIndex);
  assert(ctrlIndex < _ctrlCount);
  LineKM line(-1, _ctrlCount, 0.0, 360.0);
  return Angle<double>::degrees(line(ctrlIndex));
}

} /* namespace mmm */
