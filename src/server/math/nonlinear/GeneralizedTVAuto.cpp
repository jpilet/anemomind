/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GeneralizedTVAuto.h"

namespace sail {

GeneralizedTVAuto::GeneralizedTVAuto(const GeneralizedTV &tv,
  double initValue = 1.0, double step = 1.0, int maxIter = 4) :
  _tv(tv), _initValue(initValue), _step(step), _maxIter(maxIter),
  _factor(log(2)) {}

UniformSamples GeneralizedTVAuto::filter(UniformSamples initialSignal,
                Arrayd X, Arrayd Y,
                int order,
                Array<Arrayb> splits) const {
  return UniformSamples();
}

UniformSamples GeneralizedTVAuto::filter(Arrayd Y, int order, Array<Arrayb> splits) const {
  return UniformSamples();
}



}
