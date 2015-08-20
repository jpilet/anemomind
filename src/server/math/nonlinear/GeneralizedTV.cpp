/*
 *  Created on: 2014-10-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cmath>
#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/math/BandMat.h>
#include <server/common/logging.h>
#include <server/common/ScopedLog.h>
#include <server/common/Span.h>
#include <server/common/ArrayBuilder.h>

namespace sail {


GeneralizedTV::GeneralizedTV(int iters, double minv, double gaussElimTol) {
   _settings.iters = iters;
   _settings.residualLowerBound = minv;
   _settings.tol = gaussElimTol;
}

Array<Observation<1> > makeObservations(Sampling s, Arrayd X, Arrayd Y) {
  int n = X.size();
  ArrayBuilder<Observation<1> > observations(n);
  for (int i = 0; i < n; i++) {
    auto w = s.represent(X[i]);
    if (s.valid(w)) {
      auto obs =  Observation<1>{w, {Y[i]}};
      observations.add(obs);
    }
  }
  return observations.get();
}

UniformSamplesd GeneralizedTV::filter(UniformSamplesd initialSignal,
                Arrayd X,
                Arrayd Y,
                int order,
                double regularization) const {
  CHECK(X.size() == Y.size());

  auto settings = _settings;
  settings.regOrder = order;
  settings.lambda = regularization;
  Sampling sampling = initialSignal.sampling();
  MDArray2d initialX(sampling.count(), 1, initialSignal.samples());
  auto Xopt = BandedSolver::solve(SquareCost(), AbsCost(), sampling,
      makeObservations(sampling, X, Y), settings, initialX);
  return UniformSamplesd(sampling, Xopt.getStorage());
}

UniformSamplesd GeneralizedTV::filter(Arrayd X, Arrayd Y, double samplingDensity,
                  int order,
                  double regularization) const {
  return filter(makeInitialSignal(X, Y, samplingDensity), X, Y, order, regularization);
}


UniformSamplesd GeneralizedTV::makeInitialSignal(Arrayd Y) {
  int count = Y.size();
  Arrayd samples(count+1);
  Y.copyToSafe(samples.sliceBut(1));
  samples.last() = Y.last();
  return UniformSamplesd(LineKM(1.0, -0.5), samples);
}

namespace {

}

UniformSamplesd GeneralizedTV::makeInitialSignal(Arrayd X, Arrayd Y, double samplingDensity) {
  assert(X.size() == Y.size());
  int obsCount = X.size();

  Spand xspan(X);
  Spand sp = xspan.getWider(1.0e-3*xspan.width());
  assert(sp.minv() < xspan.minv());
  assert(xspan.maxv() < sp.maxv());
  int sampleCount = std::max(2, int(round(sp.width()/samplingDensity)));
  assert(sampleCount >= 2);
  LineKM ind2x(0, sampleCount-1, sp.minv(), sp.maxv());
  Arrayd initialSignal(sampleCount);
  BandMatd AtA(sampleCount, sampleCount, 2, 2);
  AtA.addRegs(Arrayi::args(2), Arrayd::args(1.0));
  assert(AtA(0, 0) > 1.0e-6);
  MDArray2d AtB(sampleCount, 1);
  AtB.setAll(0);
  for (int i = 0; i < obsCount; i++) {
    int I[2];
    double W[2];
    ind2x.makeInterpolationWeights(X[i], I, W);
    AtA.addNormalEq(2, I, W);
    AtB(I[0], 0) += W[0]*Y[i];
    AtB(I[1], 0) += W[1]*Y[i];
  }
  assert(bandMatGaussElimDestructive(&AtA, &AtB, 1.0e-12));
  Arrayd data = AtB.getStorage();
  assert(data.size() == sampleCount);
  return UniformSamplesd(ind2x, data);
}

Arrayd GeneralizedTV::makeDefaultX(int size) {
  return Arrayd::fill(size, [](int i) {return double(i);});
}

UniformSamplesd GeneralizedTV::filter(Arrayd Y, int order,
    double regularization) const {
  return filter(makeInitialSignal(Y),
      makeDefaultX(Y.size()), Y, order, regularization);
}


}
