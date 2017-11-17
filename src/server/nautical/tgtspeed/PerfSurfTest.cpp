/*
 * PerfSurfTest.cpp
 *
 *  Created on: 12 Nov 2017
 *      Author: jonas
 */

#include <server/nautical/tgtspeed/PerfSurf.h>
#include <random>
#include <server/common/math.h>

using namespace sail;

auto unit = 1.0_kn;

// A speed function that given wind speed returns the maximum boat
// speed. Just hypothetical, for testing.
Velocity<double> trueMaxSpeed(const Velocity<double>& src) {
  return unit*10.0*(1 - exp(double(-src/5.0_kn)));
}

std::default_random_engine rng(0);

// Returns a function that given a value, produces a random value
// close to that value and within bounds. Random walk.
//
// We will use that to produce random wind and performance values,
// with some temporal continuity.
std::function<double(double)> smoothGen(double maxVal, double maxChange) {
  auto distrib = std::make_shared<
      std::uniform_real_distribution<double>>(-maxChange, maxChange);
  return [maxVal, distrib](double x) {
    return clamp<double>(x + (*distrib)(rng), 0, maxVal);
  };
}

Array<double> makeData(int n) {
  double perf = 0;
  Velocity<double> maxWind = 15.0_mps;
  Velocity<double> wind = 0.0_kn;
  TimeStamp offset = TimeStamp::UTC(2017, 11, 12, 15, 1, 0);
  Duration<double> stepSize = 0.1_s;
  Array<PerfSurfPt> pts(n);
  for (int i = 0; i < n; i++) {
    TimeStamp time = offset + double(i)*stepSize;

    auto maxSpeed = trueMaxSpeed(wind);

    PerfSurfPt pt;
    pt.performance = perf;
    pt.speed = perf*maxSpeed;
  }
  return n;
}
