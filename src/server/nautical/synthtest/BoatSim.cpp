/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatSim.h"
#include <cassert>
#include <server/common/LineKM.h>
#include <server/common/SharedPtrUtils.h>
#include <server/common/PhysicalQuantityIO.h>

namespace sail {


namespace {
  double testTargetSpeedProfile(Angle<double> twa) {
    if (cos(twa) > 0) {
      return std::abs(sin(twa));
    } else {
      return LineKM(-1.0, 1.0, 1.0, 0.8)(cos(twa.scaled(2.0)));
    }
  }
}

Velocity<double> BoatCharacteristics::defaultTargetSpeed(Angle<double> twa, Velocity<double> tws) {
  return 0.5*testTargetSpeedProfile(twa)*tws;
}

BoatSimulator::BoatSimulator(
    FlowFun windFun,
    FlowFun currentFun,
    BoatCharacteristics ch,
    Array<TwaDirective> twaSpans) :
        _windFun(windFun),
        _currentFun(currentFun),
        _ch(ch), _twaSpans(twaSpans) {
  _indexer = ProportionateIndexer(twaSpans.map<double>([&](TwaDirective d) {
    return d.duration.seconds();
  }));
}




BoatSimulator::FullBoatState BoatSimulator::makeFullState(const BoatSimulationState &state) {
  FullBoatState dst;
  dst.rudderAngle = Angle<double>::radians(state.rudderAngleRadians);
  dst.x = Length<double>::meters(state.boatXMeters);
  dst.y = Length<double>::meters(state.boatYMeters);
  dst.time = Duration<double>::seconds(state.timeSeconds);
  dst.boatOrientation = Angle<double>::radians(state.boatOrientationRadians);
  dst.boatSpeedThroughWater = Velocity<double>::metersPerSecond(state.boatSpeedThroughWaterMPS);

  dst.trueWind = _windFun(dst.x, dst.y, dst.time);
  dst.trueCurrent = _currentFun(dst.x, dst.y, dst.time);

  // Since a polar table will assume no current, it makes sense
  // to compute the "true" wind in a coordinate system attached to
  // the local water surface.
  dst.windWrtCurrent = dst.trueWind - dst.trueCurrent;

  dst.twaWater = (dst.windWrtCurrent.angle() + Angle<double>::radians(M_PI)) - dst.boatOrientation;
  dst.twsWater = dst.windWrtCurrent.norm();

  dst.boatMotionThroughWater =
      HorizontalMotion<double>::polar(dst.boatSpeedThroughWater, dst.boatOrientation);

  dst.boatMotion = dst.trueCurrent + dst.boatMotionThroughWater;

  return dst;
}


void BoatSimulator::eval(double *Xin, double *Fout, double *Jout) {
  assert(Jout == nullptr);
  BoatSimulationState &state = *((BoatSimulationState *)Xin);
  BoatSimulationState &deriv = *((BoatSimulationState *)Fout);
  FullBoatState full = makeFullState(state);


  double twaAngleErrorRadians = (getTargetTwa(full.time) - full.twaWater).radians();
  Angle<double> targetRudderAngle = Angle<double>::radians(0);
  if (std::abs(twaAngleErrorRadians) > _ch.correctionThreshold.radians()) {
    targetRudderAngle = (twaAngleErrorRadians > 0? 1.0 : -1.0)*_ch.rudderMaxAngle;
  }

  // COMPUTE THE DERIVATIVES THAT TELL HOW THE STATE
  // VECTOR WILL EVOLVE.

  // The boat strives to reach its target speed, but is slowed down when the rudder angle is nonzero.
  deriv.boatSpeedThroughWaterMPS =
      _ch.targetSpeedGain()*(
          _ch.targetSpeedFun(full.twaWater, full.twsWater).metersPerSecond() - state.boatSpeedThroughWaterMPS)
        - std::abs(_ch.rudderResistanceCoef*sin(full.rudderAngle))*sqr(state.boatSpeedThroughWaterMPS);

  // The derivative of the boat X and Y positions is the boat motion.
  deriv.boatXMeters = full.boatMotion[0].metersPerSecond();
  deriv.boatYMeters = full.boatMotion[1].metersPerSecond();

  // For a positive rudder angle, the boat orientation will decrease. The faster the boat is moving forward
  // also, the faster the boat will turn. The greater the distance between the rudder and the keel,
  // the slower the boat will turn.
  deriv.boatOrientationRadians = -full.boatSpeedThroughWater.metersPerSecond()*sin(full.rudderAngle)
      /(_ch.keelRudderDistance.meters());

  // The helmsman seeks to approach a target angle of the rudder.
  deriv.rudderAngleRadians = _ch.rudderCorrectionCoef*(targetRudderAngle - full.rudderAngle).radians();

  // The derivative of time w.r.t. time is 1.
  deriv.timeSeconds = 1.0;
}

Array<BoatSimulator::FullBoatState> BoatSimulator::simulate(Duration<double> simulationDuration,
  Duration<double> samplingPeriod, int iterationsPerSample) {
  int sampleCount = int(simulationDuration/samplingPeriod);
  double stepsize = samplingPeriod.seconds()/iterationsPerSample;
  Array<FullBoatState> dst(sampleCount);
  BoatSimulationState state;
  Arrayd stateVector(state.paramCount(), (double *)(&state));
  RungeKutta rk(makeSharedPtrToStack(*this));
  for (int i = 0; i < sampleCount; i++) {
    dst[i] = makeFullState(state);
    for (int j = 0; j < iterationsPerSample; j++) {
      rk.step(&stateVector, stepsize);
    }
  }
  return dst;
}

std::ostream &operator<<(std::ostream &s,
    const BoatSimulator::FullBoatState &state) {
  s << EXPR_AND_VAL_AS_STRING(state.boatMotion) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.boatMotionThroughWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.boatOrientation) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.boatSpeedThroughWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.rudderAngle) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.time) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.trueCurrent) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.trueWind) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.twaWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.twsWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.windWrtCurrent) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.x) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.y) << std::endl;
  return s;
}



}
