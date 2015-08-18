/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_COMMON_SAMPLING_H_
#define SERVER_COMMON_SAMPLING_H_

#include <server/common/LineKM.h>

namespace sail {

class Sampling {
 public:
  Sampling() : sampleCount(0) {}
  Sampling(int count, double lower, double upper);

  struct Weights {
   int lowerIndex;
   double lowerWeight;

   int upperIndex() const {
     return lowerIndex + 1;
   }

   double upperWeight() const {
     return 1.0 - lowerWeight;
   }
  };

  Weights represent(double x) const;

  const LineKM &indexToX() const {
    return _indexToX;
  }

  const LineKM &xToIndex() const {
    return _xToIndex;
  }
 private:
  LineKM _indexToX, _xToIndex;
  int sampleCount;
};

} /* namespace sail */

#endif /* SERVER_COMMON_SAMPLING_H_ */
