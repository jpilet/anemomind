/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PolarOptimizer.h"
#include <cassert>
#include <server/math/ADFunction.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/armaadolc.h>

namespace sail {



namespace {
  class Objf : public AutoDiffFunction {
   public:
    Objf(const PolarSurfaceParam &param,
         const PolarDensity &density,
         Array<Vectorize<double, 2> > pts) :
      _param(param), _density(density), _surfpts(pts) {}

    int inDims() {
      return _param.paramCount();
    }

    int outDims() {
      return _surfpts.size();
    }
    void evalAD(adouble *Xin, adouble *Fout);
   private:
    Array<Vectorize<double, 2> > _surfpts;
    const PolarSurfaceParam &_param;
    const PolarDensity &_density;
  };

  void Objf::evalAD(adouble *Xin, adouble *Fout) {
    Arrayad X(inDims(), Xin);
    Arrayad vertices(_param.vertexDim());
    _param.paramToVertices(X, vertices);

    int count = _surfpts.size();
    assert(count == outDims());
    for (int i = 0; i < count; i++) {
      Vectorize<Velocity<adouble>, 3> surfpt
        = _param.computeSurfacePoint(vertices, _surfpts[i]);
      adouble f = _density.lsqResidue(surfpt.data());
      assert(0 < f.getValue());
      Fout[i] = f;
    }
  }
}

Arrayd optimizePolar(const PolarSurfaceParam &param, const PolarDensity &density,
    Array<Vectorize<double, 2> > surfpts,
  Arrayd initParams,
  LevmarSettings settings) {

  Arrayd params = (initParams.empty()? param.makeInitialParams() : initParams);
  LevmarState state(params);

  Objf fun(param, density, surfpts);
  state.minimize(settings, fun);
  return state.getXArray();
}


} /* namespace mmm */
