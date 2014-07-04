/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/HorizontalMotionParam.h>
#include <server/common/Uniform.h>

namespace sail {

void ParametricHorizontalMotion::initializeRandom(double *outParams) const {
  Uniform rng(-16, 16);
  outParams[0] = rng.gen();
  outParams[1] = rng.gen();
}


}


