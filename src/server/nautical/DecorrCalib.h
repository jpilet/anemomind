/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DECORRCALIB_H_
#define DECORRCALIB_H_

#include <server/nautical/Corrector.h>
#include <server/nautical/FilteredNavData.h>

namespace sail {

/*
 * Calibration by assuming that wind and current are locally decorrelated.
 * This is reasonable. A sudden change in the wind is likely not to affect the current,
 * because the current has so much inertia. For longer time spans of several hours,
 * this decorrelation assumption may be less true.
 *
 */
class DecorrCalib {
 public:
  DecorrCalib() :
    _normalized(false),
    _windowSize(30),
    _polyDeg(1),
    _windowOverlap(0.5) {}

  void setNormalized(bool n) {
    _normalized = n;
  }

  void setWindowSize(int windowSize) {
    _windowSize = windowSize;
  }

  void setPolyDeg(int polyDeg) {
    _polyDeg = polyDeg;
  }

  void setWindowOverlap(double overlap) {
    assert(0 <= overlap);
    assert(overlap <= 1);
    _windowOverlap = overlap;
  }

  class Results {
   public:
    Results(const Corrector<double> &c,
      FilteredNavData d) : corrector(c), data(d) {}

    Corrector<double> corrector;
    FilteredNavData data;

  };

  Results calibrate(FilteredNavData data);



  bool normlalized() const {
    return _normalized;
  }

  int windowSize() const {
    return _windowSize;
  }

  int polyDeg() const {
    return _polyDeg;
  }

  double relativeWindowOverlap() const {
    return _windowOverlap;
  }

  int absoluteWindowOverlap() const {
    return std::max(1, int(floor((1.0 - _windowOverlap)*_windowSize)));
  }
 private:
  /*
   * Whether the covariance should be normalized or not.
   * Normalized covariance is called correlation.
   */
  bool _normalized;

  /*
   * How large windows in time we should compute the covariance across.
   * A larger window means more data for estimating the covariance, but
   * also means that the linearity assumption gets weeker.
   */
  int _windowSize;

  /*
   * Degree of the polynom that is subtracted from a signal before covariance
   * is computed. Degree 0 means that the mean is subtracted which is the common case.
   * But maybe it is more appropriate to subtract a line fitted in the least squares sense.
   */
  int _polyDeg;

  /*
   * How much two windows overlap.
   */
  double _windowOverlap;
};

}

#endif /* DECORRCALIB_H_ */
