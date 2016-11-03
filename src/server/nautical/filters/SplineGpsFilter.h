/*
 * SplineGpsFilter.h
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_SPLINEGPSFILTER_H_
#define SERVER_NAUTICAL_FILTERS_SPLINEGPSFILTER_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/nautical/Ecef.h>
#include <server/common/TimeMapper.h>
#include <server/math/Spline.h>
#include <server/common/TimedValue.h>
#include <server/nautical/GeographicPosition.h>
#include <server/math/BandedLevMar.h>
#include <server/math/OutlierRejector.h>
#include <server/common/MDArray.h>

namespace sail {
namespace SplineGpsFilter {

struct Settings {
  Duration<double> period = 2.0_s;
  Length<double> inlierThreshold = 12.0_m;
  double regWeight = 1.0;
  double stabilizerWeight = 1.0e-12;
  BandedLevMar::Settings lmSettings;

  double initialWeight = 0.01;
  double finalWeight = 1000;

  std::function<double(int)> weightToIndex() const;

  OutlierRejector::Settings positionSettings() const;
};

class EcefCurve {
public:
  EcefCurve() {}
  EcefCurve(
      const TimeMapper &mapper,
      const SmoothBoundarySplineBasis<double, 3> &b,
      const double *coefs, int stride);

  TimeStamp lower() const;
  TimeStamp upper() const;
  ECEFCoords<double> evaluateEcefPosition(TimeStamp t) const;
  ECEFCoords<double, 1> evaluateEcefMotion(TimeStamp t) const;

  GeographicPosition<double> evaluateGeographicPosition(TimeStamp t) const;
  HorizontalMotion<double> evaluateHorizontalMotion(TimeStamp t) const;
private:
  TimeMapper _mapper;
  SmoothBoundarySplineBasis<double, 3> _basis, _motionBasis;
  Array<double> _coefs[3];
};

struct Curve {
  TimeMapper timeMapper;
  SmoothBoundarySplineBasis<double, 3> basis;

  Span<TimeStamp> timeSpan() const {
    return Span<TimeStamp>(
        timeMapper.unmap(basis.raw().lowerDataBound()),
        timeMapper.unmap(basis.raw().upperDataBound()));
  }

  Curve() {}
  Curve(const TimeMapper &mapper) : timeMapper(mapper),
      basis(mapper.sampleCount) {}

};

Array<Curve> filter(
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    const Array<TimeMapper> &segments,
    Settings settings);

}
}

#endif /* SERVER_NAUTICAL_FILTERS_SPLINEGPSFILTER_H_ */
