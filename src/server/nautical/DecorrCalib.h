/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DECORRCALIB_H_
#define DECORRCALIB_H_

#include <server/nautical/Corrector.h>
#include <server/nautical/FilteredNavData.h>
#include <random>

namespace sail {

Corrector<double> makeRandomCorrector(std::default_random_engine &e);
Array<Corrector<double> > makeRandomCorrectors(int count, std::default_random_engine &e);
Array<Corrector<double> > makeCorruptCorrectors();

/*
 * Calibration by assuming that wind and current are locally decorrelated.
 * This is reasonable. A sudden change in the wind is likely not to affect the current,
 * because the current has so much inertia. For longer time spans of several hours,
 * this decorrelation assumption may be less true.
 *
 * How it works:
 * The correlation between the actual wind and a corrupted current is close to 0. The
 * correlation between the actual current and a corrupted wind is also close to 0. This
 * is what we minimize over many short overlapping time windows.
 *
 */
class DecorrCalib {
 public:
  DecorrCalib() :
    _normalized(true),
    _withSqrt(true),
    _windowSize(30),
    _polyDeg(0),
    _windowOverlap(0.5) {}

  void setNormalized(bool n) {
    _normalized = n;
  }

  bool withSqrt() const {
    return _withSqrt;
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
    Results() {}

    Results(const Corrector<double> &c,
      FilteredNavData d) : corrector(c), data(d) {}

    Corrector<double> corrector;
    FilteredNavData data;

  };

  Results calibrate(FilteredNavData data,
      Array<Corrector<double> > corrupted = Array<Corrector<double> >());



  bool normalized() const {
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

  int windowStep() const {
    return std::max(1, int(floor((1.0 - _windowOverlap)*_windowSize)));
  }
 private:
  /*
   * Whether the covariance should be normalized or not.
   * Normalized covariance is called correlation.
   */
  bool _normalized;

  /*
   * Whether the square root should be applied to the residuals.
   */
  bool _withSqrt;

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
