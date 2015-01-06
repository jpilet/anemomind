/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FRACTALFLOW_H_
#define FRACTALFLOW_H_

#include <server/nautical/synthtest/Flow.h>
#include <server/math/SubdivFractals.h>

namespace sail {

// Make a standard wind flow.

class WindCurrentPair {
 public:
  WindCurrentPair(Flow w, Flow c) : wind(w), current(c) {}
  Flow wind, current;
};

// Generate winds and currents defined over an area
// 1000x1000 nautical miles^2 and a week. Winds go up to 20 m/s
// and currents a little more than 2 knots.
WindCurrentPair makeWindCurrentPair001();



Flow makeFractalFlow(Length<double> unitLength,
                     Duration<double> unitTime,
                     Velocity<double> unitVelocity,
                     Angle<double> unitAngle,
                     SubdivFractals::Fractal<3> velocityFractal,
                     SubdivFractals::Fractal<3> angleFractal);

}

#endif /* FRACTALFLOW_H_ */
