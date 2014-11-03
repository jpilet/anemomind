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

class EdgeDetector1d {
 public:
  EdgeDetector1d(double lambda, int minimumEdgeCount = 0);

  class LineSegment {
   public:
    LineSegment() {}
    LineSegment(Spani span, LineFitQF lineFit)  :
      _span(span), _lineFit(lineFit) {}
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
