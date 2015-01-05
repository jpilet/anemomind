/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FractalFlow.h"

namespace sail {

namespace {
  Flow::VelocityFunction makeFractalFlowSub(Length<double> unitLength,
      Duration<double> unitTime,
      SubdivFractals::Fractal<3> velocityFractal,
      SubdivFractals::Fractal<3> angleFractal, double phase) {
    return [=](Flow::ProjectedPosition pos, Duration<double> time) {
      double local[3] = {pos[0]/unitLength, pos[1]/unitLength, time/unitTime};
      double angle = angleFractal.eval(local);
      double vel = velocityFractal.eval(local);
      return sin(angle + phase)*vel;
    };
  }
}

Flow makeFractalFlow(Length<double> unitLength,
    Duration<double> unitTime,
    SubdivFractals::Fractal<3> velocityFractal,
    SubdivFractals::Fractal<3> angleFractal) {

  // This is a bit inelegant, that we have two functions that both evaluate
  // more or less the same thing... Twice as much work. Maybe, if it turns out
  // that we only need flows to be defined as fractals, we can restructure the
  // code to avoid this.
  return Flow(makeFractalFlowSub(unitLength, unitTime,
                                 velocityFractal, angleFractal, 0.5*M_PI),
              makeFractalFlowSub(unitLength, unitTime,
                                 velocityFractal, angleFractal, 0.0));
}


}
