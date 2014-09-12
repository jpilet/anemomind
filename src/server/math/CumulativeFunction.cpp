/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CumulativeFunction.h"

namespace sail {


CumulativeFunction::CumulativeFunction(Arrayd samples) {
  int count = samples.size();
  _cumul = Arrayd(count);
  _cumul[0] = 0;
  for (int i = 1; i < count; i++) {
    _cumul[i] = _cumul[i-1] + 0.5*(samples[i-1] + samples[i]);
  }
  double totalSum = _cumul.last();
  double factor = 1.0/totalSum;
  for (int i = 0; i < count; i++) {
    _cumul[i] *= factor;
  }
  _k = 1.0/(count - 1);
}

} /* namespace mmm */
