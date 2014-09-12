/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PolarOptimizer.h"
#include <cassert>
#include <server/math/ADFunction.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/armaadolc.h>
#include <server/common/ScopedLog.h>

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
    ENTERSCOPE("Objf::evalAD");
    static int counter = 0;
    Arrayad X(inDims(), Xin);
    Arrayad vertices(_param.vertexDim());
    _param.paramToVertices(X, vertices);


    for (int i = 0; i < X.size(); i++) {
      assert(!std::isnan(X[i].getValue()));
    }
    for (int i = 0; i < vertices.size(); i++) {
      if (std::isnan(vertices[i].getValue())) {
        for (int i = 0; i < X.size(); i++) {
          std::cout << EXPR_AND_VAL_AS_STRING(X[i].getValue()) << std::endl;
        }
        std::cout << EXPR_AND_VAL_AS_STRING(counter) << std::endl;
        assert(false);
      }
    }


    int count = _surfpts.size();
    assert(count == outDims());
    for (int i = 0; i < count; i++) {
      Vectorize<Velocity<adouble>, 3> surfpt
        = _param.computeSurfacePoint(vertices, _surfpts[i]);
      const Velocity<adouble> *pt = surfpt.data();
      for (int i = 0; i < 3; i++) {
        assert(!std::isnan(pt[i].knots().getValue()));
      }
      adouble f = _density.lsqResidue(pt);
      double val = f.getValue();
      assert(0 < val);
      Fout[i] = f;
    }
    counter++;
  }
}

Arrayd optimizePolar(const PolarSurfaceParam &param, const PolarDensity &density,
    Array<Vectorize<double, 2> > surfpts,
  Arrayd initParams,
  LevmarSettings settings) {

  Arrayd params = (initParams.empty()? param.makeInitialParams() : initParams);
  assert(params.size() == param.paramCount());
  assert(param.paramCount() < surfpts.size());
  LevmarState state(params);

  Objf fun(param, density, surfpts);
  state.minimize(settings, fun);
  return state.getXArray();
}


} /* namespace mmm */
