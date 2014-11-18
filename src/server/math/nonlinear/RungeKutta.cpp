/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "RungeKutta.h"
#include <server/common/logging.h>
#include <cassert>

namespace sail {

RungeKutta::RungeKutta(std::shared_ptr<Function> fun) :
    _fun(fun), _dim(_fun->inDims()) {
  CHECK(_fun->inDims() == _fun->outDims());
  _k1 = Arrayd(_dim);
  _k2 = Arrayd(_dim);
  _k3 = Arrayd(_dim);
  _k4 = Arrayd(_dim);
  _temp = Arrayd(_dim);
}

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


void RungeKutta::step(Arrayd *stateVector, double stepSize) {
  assert(stateVector->size() == _dim);
  _fun->eval(stateVector->ptr(), _k1.ptr());
  addWeighted(*stateVector, 0.5*stepSize, _k1, _temp);
  _fun->eval(_temp.ptr(), _k2.ptr());
  addWeighted(*stateVector, 0.5*stepSize, _k2, _temp);
  _fun->eval(_temp.ptr(), _k3.ptr());
  addWeighted(*stateVector, stepSize, _k3, _temp);
  _fun->eval(_temp.ptr(), _k4.ptr());
  double factor = stepSize/6;
  for (int i = 0; i < _dim; i++) {
    (*stateVector)[i] += factor*(_k1[i] + 2*(_k2[i] + _k3[i]) + _k4[i]);
  }
}


}
