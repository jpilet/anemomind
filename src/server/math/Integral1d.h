/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef INTEGRAL1D_H_
#define INTEGRAL1D_H_

#include <server/common/Array.h>
#include <server/common/Span.h>

namespace sail {

template <typename T>
class Integral1d {
 public:
  Integral1d() {}
  Integral1d(Array<T> data, T offset = T(0)) {
    int srcSize = data.size();
    _integral = Array<T>(srcSize + 1);
    _integral[0] = offset;
    for (int i = 0; i < srcSize; i++) {
      _integral[i + 1] = _integral[i] + data[i];
    }
  }

  T integrate(int from, int to) const {
    return _integral[to] - _integral[from];
  }

  T integrate(Spani span) const {
    return integrate(span.minv(), span.maxv());
  }

  T average(int from, int to) const {
    return (1.0/(to - from))*integrate(from, to);
  }

  T integral() const {
    return integrate(0, size());
  }

  // Size of the source array over which we are integrating.
  int size() const {
    return _integral.size() - 1;
  }

  Integral1d<T> slice(int from, int to) const {
    Integral1d<T> dst;
    dst._integral = _integral.slice(from, to+1);
    return dst;
  }

  Integral1d<T> sliceFrom(int from) const {
    return slice(from, size());
  }

  Integral1d<T> sliceTo(int to) const {
    return slice(0, to);
  }

  const Array<T> &data() const {
    return _integral;
  }
 private:
  Array<T> _integral;
};


}



#endif /* INTEGRAL1D_H_ */
