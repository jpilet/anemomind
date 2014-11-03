/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef EDGEDETECTOR1D_H_
#define EDGEDETECTOR1D_H_

#include <server/common/LineKM.h>
#include <server/math/QuadForm.h>
#include <server/common/Span.h>

namespace sail {

/*
 * Given measurements X for which we have corresponding measurements Y,
 * fit a signal consisting of piecewise straight lines.
 * The 'lambda' parameter controls the regularization:
 * A higher value of lambda means fewer line segments.
 *
 * Formally, an objective f(LineSegments) is minimized:
 *
 *   f(LineSegments) = Data(LineSegments) + lambda*NumberOfDiscontinuities(LineSegments)
 *
 * where 'Data(LineSegments)' is the least-squares fit to the data.
 *
 *
 */
class LineFitter {
 public:
  LineFitter(double lambda, int minimumEdgeCount = 0);

  class LineSegment {
   public:
    LineSegment() {}
    LineSegment(Spani span_, LineFitQF lineFit_)  :
      _span(span_), _lineFit(lineFit_) {}

    Spani span() const {
      return _span;
    }

    const LineFitQF &lineFit() const {
      return _lineFit;
    }
   private:
    Spani _span;
    LineFitQF _lineFit;
  };

  Array<LineSegment> detect(LineKM sampling, int sampleCount, Arrayd X, Arrayd Y) const;
 private:
  double _lambda;
  int _minimumEdgeCount;
};

}

#endif /* EDGEDETECTOR1D_H_ */
