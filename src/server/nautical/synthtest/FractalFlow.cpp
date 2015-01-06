/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FractalFlow.h"
#include <random>

namespace sail {

namespace {
  Flow::VelocityFunction makeFractalFlowSub(Length<double> unitLength,
      Duration<double> unitTime, Velocity<double> unitVelocity,
      SubdivFractals::Fractal<3> velocityFractal,
      SubdivFractals::Fractal<3> angleFractal, double phase) {
    return [=](Flow::ProjectedPosition pos, Duration<double> time) {
      double local[3] = {pos[0]/unitLength, pos[1]/unitLength, time/unitTime};
      double angle = angleFractal.evalOrtho(local);
      double vel = velocityFractal.evalOrtho(local);
      return sin(angle + phase)*vel*unitVelocity;
    };
  }
}

Flow makeFractalFlow(Length<double> unitLength,
    Duration<double> unitTime, Velocity<double> unitVelocity,
    SubdivFractals::Fractal<3> velocityFractal,
    SubdivFractals::Fractal<3> angleFractal) {

  // This is a bit inelegant, that we have two functions that both evaluate
  // more or less the same thing... Twice as much work. Maybe, if it turns out
  // that we only need flows to be defined as fractals, we can restructure the
  // code to avoid this.
  return Flow(makeFractalFlowSub(unitLength, unitTime, unitVelocity,
                                 velocityFractal, angleFractal, 0.5*M_PI),
              makeFractalFlowSub(unitLength, unitTime, unitVelocity,
                                 velocityFractal, angleFractal, 0.0));
}

using namespace SubdivFractals;

namespace {
  Array<Vertex> makeRandomCtrl(double maxv, int classCount,
    std::default_random_engine &e) {
    std::uniform_real_distribution<double> value(-maxv, maxv);
    std::uniform_int_distribution<int> index(0, classCount-1);
    const int count = Fractal<3>::ctrlCount;
    Array<Vertex> vertices(count);
    for (int i = 0; i < count; i++) {
      vertices[i] = Vertex(value(e), index(e));
    }
    return vertices;
  }


}



Flow makeWindFlow001() {

  std::default_random_engine e;

  Length<double> unitLength = Length<double>::nauticalMiles(1000);
  Duration<double> unitTime = Duration<double>::days(7);

  // Tuned so that the wind is in range.
  Velocity<double> unitVelocity = Velocity<double>::metersPerSecond(1.0);

  int depth = 25;



  int classCount = 4;

  double angleBd = 0.1;
  MaxSlope angleMaxSlope(angleBd, 1.0);
  MDArray<Rule::Ptr, 2> angleRules = makeRandomBoundedRules(classCount, angleMaxSlope, e);

  double bd = 0.1;
  MaxSlope velMaxSlope(bd, 1.0);
  MDArray<Rule::Ptr, 2> velRules = makeRandomBoundedRules(classCount, velMaxSlope, e);

  double maxAngle;
  Array<Vertex> angleCtrl = makeRandomCtrl(angleBd, classCount, e);
  Array<Vertex> velCtrl = makeRandomCtrl(bd, classCount, e);


  return makeFractalFlow(unitLength, unitTime, unitVelocity,
      Fractal<3>(velRules, velCtrl, depth),
      Fractal<3>(angleRules, angleCtrl, depth));

}


}
