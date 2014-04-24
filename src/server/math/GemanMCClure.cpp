/*
 *  Created on: 2014-04-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GemanMCClure.h"
#include <server/common/invalidate.h>
#include <assert.h>
#include <server/common/math.h>

namespace sail {

GemanMCClure::GemanMCClure() : _iters(0) {
  InvalidateScalar(&_sigma2);
}


GemanMCClure::GemanMCClure(double sigma, double initR, int count) :
  _sigma2(sqr(sigma)), _iters(1) {
  _R2sum = Arrayd::fill(count, sqr(initR));
}

void GemanMCClure::addResiduals(Arrayd residuals) {
  int count = _R2sum.size();
  assert(count == residuals.size());
  for (int i = 0; i < count; i++) {
    _R2sum[i] += sqr(residuals[i]);
  }
  _iters++;
}


} /* namespace sail */
