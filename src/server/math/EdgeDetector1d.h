/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef EDGEDETECTOR1D_H_
#define EDGEDETECTOR1D_H_

#include <server/common/LineKM.h>
#include <server/math/QuadForm.h>

namespace sail {

class EdgeDetector1d {
 public:
  EdgeDetector1d(double lambda, int minimumEdgeCount = 0);

  class Result {
   public:
  };

  Result detect(LineKM sampling, int sampleCount, Arrayd X, Arrayd Y) const;
 private:
  double _lambda;
  int _minimumEdgeCount;
};

}

#endif /* EDGEDETECTOR1D_H_ */
