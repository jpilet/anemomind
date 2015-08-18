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
  Sampling() : _sampleCount(0) {}
  Sampling(int count, double lower, double upper);

  struct Weights {
   int lowerIndex;
   double lowerWeight, upperWeight;

   int upperIndex() const {
     return lowerIndex + 1;
   }
  };

  Weights represent(double x) const;

  const LineKM &indexToX() const {
    return _indexToX;
  }

  const LineKM &xToIndex() const {
    return _xToIndex;
  }

  double period() const {
    return _indexToX.getK();
  }

  int count() const {
    return _sampleCount;
  }
 private:
  LineKM _indexToX, _xToIndex;
  int _sampleCount;
};

template <int N>
struct Observation {
  Sampling::Weights weights;
  double data[N];
};

} /* namespace sail */

#endif /* SERVER_COMMON_SAMPLING_H_ */
