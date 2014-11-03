/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef INTEGRAL1D_H_
#define INTEGRAL1D_H_

namespace sail {

template <typename T>
class Integral1d {
 public:
  Integral1d() {}
  Integral1d(Array<T> data) {
    int count = data.size();
    _integral = Array<T>(count + 1);
    _integral[0] = T(0);
    for (int i = 0; i < count; i++) {
      _integral[i + 1] = _integral[i] + data[i];
    }
  }

  T integrate(int from, int to) const {
    return _integral[to] - _integral[from];
  }
 private:
  Array<T> _integral;
};


}



#endif /* INTEGRAL1D_H_ */
