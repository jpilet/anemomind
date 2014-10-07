/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLAROPTIMIZER_H_
#define POLAROPTIMIZER_H_

#include <server/common/Array.h>
#include <server/nautical/polar/PolarSurfaceParam.h>
#include <server/nautical/polar/PolarDensity.h>
#include <server/math/nonlinear/LevmarSettings.h>

namespace sail {

Arrayd optimizePolar(const PolarSurfaceParam &param, const PolarDensity &density,
    Array<Vectorize<double, 2> > surfpts,
    Arrayd initParams = Arrayd(),
    LevmarSettings settings = LevmarSettings());


}

#endif /* POLAROPTIMIZER_H_ */
