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

struct SmoothGen {
  SmoothGen(double maxVal, double maxChange)
    : _distrib(-maxChange, maxChange),
      _maxVal(maxVal) {}

  double operator()(double x) {
    return clamp<double>(x + _distrib(rng), 0, _maxVal);
  }
private:
  std::uniform_real_distribution<double> _distrib;
  double _maxVal;
};

Array<double> makeData(int n) {
  double perf = 0;
  Velocity<double> maxWind = 15.0_mps;
  Velocity<double> wind = 0.0_kn;
  TimeStamp offset = TimeStamp::UTC(2017, 11, 12, 15, 1, 0);
  Duration<double> stepSize = 0.1_s;
  Array<PerfSurfPt> pts(n);
  SmoothGen perfGen(1.0, 0.05);
  SmoothGen windGen(16.0_mps/unit, 0.1_mps/unit);
  for (int i = 0; i < n; i++) {
    TimeStamp time = offset + double(i)*stepSize;

    auto maxSpeed = trueMaxSpeed(wind);

    PerfSurfPt pt;
    pt.performance = perf;
    pt.speed = perf*maxSpeed;

    perf = perfGen(perf);
    wind = windGen(wind/unit)*unit;
  }
  return n;
}
