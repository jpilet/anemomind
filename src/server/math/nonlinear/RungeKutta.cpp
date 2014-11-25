/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "RungeKutta.h"
#include <server/common/logging.h>
#include <cassert>

namespace sail {


namespace {
  void addWeighted(const Arrayd &a, double lambda, const Arrayd &b, Arrayd dst) {
    int dim = a.size();
    assert(dim == b.size());
    assert(dim == dst.size());
    for (int i = 0; i < dim; i++) {
      dst[i] = a[i] + lambda*b[i];
    }
  }
}


void takeRungeKuttaStep(std::shared_ptr<Function> fun, Arrayd *stateVector, double stepSize) {
  int dim = fun->inDims();
  Arrayd k1(dim), k2(dim), k3(dim), k4(dim), temp(dim);

  assert(stateVector->size() == dim);
  fun->eval(stateVector->ptr(), k1.ptr());
  addWeighted(*stateVector, 0.5*stepSize, k1, temp);
  fun->eval(temp.ptr(), k2.ptr());
  addWeighted(*stateVector, 0.5*stepSize, k2, temp);
  fun->eval(temp.ptr(), k3.ptr());
  addWeighted(*stateVector, stepSize, k3, temp);
  fun->eval(temp.ptr(), k4.ptr());
  double factor = stepSize/6;
  for (int i = 0; i < dim; i++) {
    (*stateVector)[i] += factor*(k1[i] + 2*(k2[i] + k3[i]) + k4[i]);
  }
}


}
