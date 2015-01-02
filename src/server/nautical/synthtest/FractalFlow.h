/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FRACTALFLOW_H_
#define FRACTALFLOW_H_

#include <server/nautical/synthtest/Flow.h>
#include <server/math/SubdivFractals.h>

namespace sail {

Flow makeFractalFlow(Length<double> spaceUnit,
                     Duration<double> timeUnit,
                     SubdivFractals::FractalFunction<3> xFlow,
                     SubdivFractals::FractalFunction<3> yFlow);

}

#endif /* FRACTALFLOW_H_ */
