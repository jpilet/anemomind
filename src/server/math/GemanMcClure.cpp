/*
 *  Created on: 2014-04-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GemanMcClure.h"
#include <server/common/invalidate.h>
#include <assert.h>
#include <server/common/math.h>
#include <server/common/MDArray.h>

namespace sail {

GemanMcClure::GemanMcClure() : _iters(0), _dim(0) {
  InvalidateScalar(&_sigma2);
}


GemanMcClure::GemanMcClure(double sigma,
    double initR, int count, int dim) :
    _dim(dim),
  _sigma2(sqr(sigma)), _iters(1) {
  _R2sum = Arrayd::fill(count, sqr(initR));
  assert(_dim >= 1);
}

void GemanMcClure::addResiduals(Arrayd residuals) {
  int count = _R2sum.size();
  assert(_dim*count == residuals.size());
  for (int i = 0; i < count; i++) {
    _R2sum[i] += getSquaredError(i, residuals);
  }
  _iters++;
}

double GemanMcClure::calcOutlierCost() const {
  double w2sum = 0.0;
  int count = _R2sum.size();
  for (int i = 0; i < count; i++) {
    w2sum += sqrt(1.0 - getWeight(i));
  }
  return _sigma2*w2sum;
}

double GemanMcClure::getSquaredError(int index, Arrayd residuals) const {
  double *r = residuals.ptr(index*_dim);
  double s2 = 0.0;
  for (int i = 0; i < _dim; i++) {
    s2 += sqr(r[i]);
  }
  return s2;
}

Arrayb GemanMcClure::inliers(Arrayd residuals) const {
  int count = _R2sum.size();
  assert(_dim*count == residuals.size());
  Arrayb dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = isInlier(i, residuals);
  }
  return dst;
}

namespace {
  int getCount(int funOutDims, int dim) {
    int count = funOutDims/dim;
    assert(count*dim == funOutDims);
    return count;
  }
}

GemanMcClureFunction::GemanMcClureFunction(double sigma,
    double initR, int dim,
    std::shared_ptr<Function> fun) : _gmc(sigma, initR,
        getCount(fun->outDims(), dim), dim),
    _fun(fun), _initialized(false) {}

void GemanMcClureFunction::evalNoJacobian(double *Xin, double *Fout) {
  _fun->eval(Xin, Fout, nullptr);
  int count = _fun->outDims();
  for (int i = 0; i < count; i++) {
    Fout[i] *= _gmc.getResidualWeight(i);
  }
  Fout[count] = _gmc.calcOutlierResidual();
}

void GemanMcClureFunction::evalWithJacobian(double *Xin,
    double *Fout, double *Jout) {
  assert(Fout != nullptr);
  int in = _fun->inDims();
  int out = _fun->outDims();
  MDArray2d Jfun(out, in);
  _fun->eval(Xin, Fout, Jfun.ptr());
  MDArray2d Jdst(outDims(), inDims(), Jout);

  // Don't accumulate the residuals for the first iteration.
  if (_initialized) {
    _gmc.addResiduals(Arrayd(out, Fout));
  }

  // Apply the reweighting
  for (int i = 0; i < out; i++) {
    double w = _gmc.getResidualWeight(i);
    Fout[i] *= w;
    for (int j = 0; j < in; j++) {
      Jdst(i, j) = w*Jfun(i, j);
    }
  }

  // Finally: The outlier cost
  Fout[out] = _gmc.calcOutlierResidual();
  Jdst.sliceRow(out).setAll(0.0);

  _initialized = true;
}

void GemanMcClureFunction::eval(double *Xin, double *Fout, double *Jout) {
  if (Jout == nullptr) {
    evalNoJacobian(Xin, Fout);
  } else {
    evalWithJacobian(Xin, Fout, Jout);
  }
}

Arrayb GemanMcClureFunction::inliers(double *Xin) {
  Arrayd F(_fun->outDims());
  _fun->eval(Xin, F.ptr());
  return gmc().inliers(F);
}

} /* namespace sail */
