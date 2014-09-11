/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARCURVEPARAM_H_
#define POLARCURVEPARAM_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/MDArray.h>

namespace sail {

/*
 * A class to parameterize polar curves, approximated by
 * line segments.
 */
class PolarCurveParam {
 public:
  PolarCurveParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored);
  Angle<double> ctrlAngle(int ctrlIndex) const;

  int ctrlToVertexIndex(int ctrlIndex) const {
    return _segsPerCtrlSpan*(1 + ctrlIndex);
  }

  int vertexCount() const {
    return lastVertex() + 1;
  }

  int paramCount() const {
    return _paramCount;
  }

  int ctrlToParamIndex(int paramIndex) const;

 private:
  MDArray2d _P;
  int _segsPerCtrlSpan, _ctrlCount, _paramCount;
  bool _mirrored;

  int lastVertex() const {
    return ctrlToVertexIndex(_ctrlCount);
  }

  Arrayi makeCtrlInds() const;
};

} /* namespace mmm */

#endif /* POLARCURVEPARAM_H_ */
