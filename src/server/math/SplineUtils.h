/*
 * SplineUtils.h
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_SPLINEUTILS_H_
#define SERVER_MATH_SPLINEUTILS_H_

#include <server/math/Spline.h>
#include <server/common/TimeStamp.h>

namespace sail {

Arrayd fitSplineCoefs(
    const SmoothBoundarySplineBasis<double, 3> &basis,
    std::function<double(int)> sampleFun);

class TemporalSplineCurve {
public:
  TemporalSplineCurve(
      Span<TimeStamp> timeSpan,
      Duration<double> period,
      Array<TimeStamp> src,
      Array<double> dst);

  double evaluate(TimeStamp t) const;
private:
  double toLocal(TimeStamp x) const;
  TimeStamp _offset;
  Duration<double> _period;
  SmoothBoundarySplineBasis<double, 3> _basis;
  Array<double> _coefs;
};

}

#endif /* SERVER_MATH_SPLINEUTILS_H_ */
