/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARCURVEPARAM_H_
#define POLARCURVEPARAM_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

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


 private:
  int _segsPerCtrlSpan, _ctrlCount, _paramCount;
  bool _mirrored;

  int paramToCtrlIndex(int paramIndex) const;

  int lastVertex() const {
    return ctrlToVertexIndex(_ctrlCount);
  }


};

} /* namespace mmm */

#endif /* POLARCURVEPARAM_H_ */
