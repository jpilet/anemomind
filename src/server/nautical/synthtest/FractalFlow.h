/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FRACTALFLOW_H_
#define FRACTALFLOW_H_

#include <server/nautical/synthtest/Flow.h>
#include <server/math/SubdivFractals.h>

namespace sail {





Flow makeFractalFlow(Length<double> unitLength,
                     Duration<double> unitTime,
                     SubdivFractals::Fractal<3> velocityFractal,
                     SubdivFractals::Fractal<3> angleFractal);

}

#endif /* FRACTALFLOW_H_ */
