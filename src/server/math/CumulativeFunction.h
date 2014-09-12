/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CUMULATIVEFUNCTION_H_
#define CUMULATIVEFUNCTION_H_

#include <server/math/armaadolc.h>
#include <server/common/ToDouble.h>
#include <server/common/LineKM.h>
#include <server/common/string.h>

namespace sail {

class CumulativeFunction {
 public:
  CumulativeFunction() : _k(NAN) {}
  CumulativeFunction(Arrayd samples);

  template <typename T>
  T eval(T x) const {
    double val = ToDouble(x);
    if (val < 0 || _cumul.size() - 1 <= val) {
      return x*_k;
    } else {
      int lower = int(floor(val));
      LineKM line(lower, lower+1,
          _cumul[lower], _cumul[lower+1]);
      return line.getK()*x + line.getM();
    }
  }
 private:
  Arrayd _cumul;
  double _k;
};

}

#endif /* CUMULATIVEFUNCTION_H_ */
