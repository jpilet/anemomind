/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatSim.h"
#include <cassert>
#include <server/common/LineKM.h>
#include <server/common/SharedPtrUtils.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/math/nonlinear/RungeKutta.h>
#include <server/common/ProportionateIndexer.h>
#include <server/plot/extra.h>
#include <server/common/logging.h>
#include <server/common/ScopedLog.h>
#include <server/common/Progress.h>
#include <limits>
#include <server/common/Functional.h>

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

BoatSim::BoatSim(
    FlowFun windFun,
    FlowFun currentFun,
    BoatCharacteristics ch,
    TwaFunction twaFunction) :
        _windFun(windFun),
        _currentFun(currentFun),
        _ch(ch), _twaFunction(twaFunction) {}

bool BoatSimulationState::valid() {
  auto p = toArray();
  for (int i = 0; i < paramCount(); i++) {
    if (!std::isfinite(p[i])) {
      return false;
    }
  }
  return true;
}


BoatSim::FullState BoatSim::makeFullState(const BoatSimulationState &state) {
  FullState dst;
  dst.rudderAngle = state.rudderAngle();
  dst.pos[0] = state.boatX();
  dst.pos[1] = state.boatY();
  dst.time = state.time();
  dst.boatOrientation = state.boatOrientation();
  dst.boatSpeedThroughWater = state.boatSpeedThroughWater();

  dst.trueWind = _windFun(dst.pos, dst.time);
  dst.trueCurrent = _currentFun(dst.pos, dst.time);

  // Since a polar table will assume no current, it makes sense
  // to compute the "true" wind in a coordinate system attached to
  // the local water surface.
  dst.windWrtCurrent = dst.trueWind - dst.trueCurrent;

  dst.windAngleWrtWater = (dst.windWrtCurrent.angle() + Angle<double>::radians(M_PI)) - dst.boatOrientation;
  dst.windSpeedWrtWater = dst.windWrtCurrent.norm();

  dst.boatMotionThroughWater =
      HorizontalMotion<double>::polar(dst.boatSpeedThroughWater, dst.boatOrientation);

  dst.boatMotion = dst.trueCurrent + dst.boatMotionThroughWater;

  dst.boatAngularVelocity = state.boatAngularVelocity();

  return dst;
}


void BoatSim::eval(double *Xin, double *Fout, double *Jout) {
  assert(Jout == nullptr);
  BoatSimulationState &state = *((BoatSimulationState *)Xin);
  assert(state.valid());
  BoatSimulationState &deriv = *((BoatSimulationState *)Fout);
  FullState full = makeFullState(state);


  double twaAngleErrorRadians = (_twaFunction(full.time) - full.windAngleWrtWater).radians();
  Angle<double> targetRudderAngle = Angle<double>::radians(0);
  int errorSign = (twaAngleErrorRadians > 0? 1.0 : -1.0);
  if (std::abs(twaAngleErrorRadians) > _ch.correctionThreshold.radians()) {
    targetRudderAngle = double(errorSign)*_ch.rudderMaxAngle;
  }

  // COMPUTE THE DERIVATIVES THAT TELL HOW THE STATE
  // VECTOR WILL EVOLVE.

  // The boat strives to reach its target speed, but is slowed down when the rudder angle is nonzero.
  deriv.setBoatSpeedThroughWaterDeriv(Velocity<double>::metersPerSecond(
      _ch.targetSpeedGain()*(
          _ch.targetSpeedFun(full.windAngleWrtWater, full.windSpeedWrtWater).metersPerSecond() - state.boatSpeedThroughWater().metersPerSecond())
        - std::abs(_ch.rudderResistanceCoef*sin(full.rudderAngle))*sqr(state.boatSpeedThroughWater().metersPerSecond())));

  // The derivative of the boat X and Y positions is the boat motion.
  deriv.setBoatXDeriv(full.boatMotion[0]);
  deriv.setBoatYDeriv(full.boatMotion[1]);

  // For a positive rudder angle, the boat orientation will decrease. The faster the boat is moving forward
  // also, the faster the boat will turn. The greater the distance between the rudder and the keel,
  // the slower the boat will turn.
  deriv.setBoatOrientationDeriv(state.boatAngularVelocity());


  Angle<double> dstAngularVel = Angle<double>::radians(-full.boatSpeedThroughWater.metersPerSecond()*sin(full.rudderAngle)
        /(_ch.keelRudderDistance.meters()));
  deriv.setBoatAngularVelocityDeriv(_ch.boatReactiveness*(dstAngularVel - state.boatAngularVelocity()));

  // The helmsman seeks to approach a target angle of the rudder.
  double rudderAngleRadians = _ch.rudderCorrectionCoef*(targetRudderAngle - full.rudderAngle).radians();
  rudderAngleRadians += errorSign*_ch.rudderFineTune.radians();
  deriv.setRudderAngleDeriv(Angle<double>::radians(rudderAngleRadians));

  // The derivative of time w.r.t. time is 1.
  deriv.setTimeDeriv(1.0);
}

Array<BoatSim::FullState> BoatSim::simulate(Duration<double> simulationDuration,
  Duration<double> samplingPeriod, int iterationsPerSample) {
  ENTER_FUNCTION_SCOPE;
  if (simulationDuration < samplingPeriod) {
    LOG(WARNING) << "Too short time for simulation";
    return Array<BoatSim::FullState>();
  }
  int sampleCount = int(simulationDuration/samplingPeriod) - 1; // subtract -1 to have some margin for round-off errors.
  double stepsize = samplingPeriod.seconds()/iterationsPerSample;
  Array<FullState> dst(sampleCount);
  BoatSimulationState state;
  Arrayd stateVector = state.toArray();
  auto fun = makeSharedPtrToStack(*this);

  Progress prog(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    dst[i] = makeFullState(state);
    for (int j = 0; j < iterationsPerSample; j++) {
      takeRungeKuttaStep(fun, &stateVector, stepsize);
    }
    if (prog.endOfIteration()) {
      SCOPEDMESSAGE(INFO, prog.iterationMessage());
    }
  }
  return dst;
}

BoatSim::TwaFunction BoatSim::makePiecewiseTwaFunction(
    Array<Duration<double> > durs,
    Array<Angle<double> > twa) {
  Arrayd dursSeconds = toArray(map(durs, [](Duration<double> x) {return x.seconds();}));
  ProportionateIndexer indexer(dursSeconds);
  return [=](Duration<double> x) {
    return twa[indexer.get(x.seconds()).index];
  };
}

namespace {
  Arrayd getTimes(Array<BoatSim::FullState> states) {
    return toArray(map(states, [=](BoatSim::FullState s) {
      return s.time.seconds();
    }));
  }


  void plotAngles(const char *title, Array<BoatSim::FullState> states,
      std::function<Angle<double>(BoatSim::FullState)> fun) {
    GnuplotExtra plot;
    plot.set_title(title);
    plot.set_style("lines");
    plot.plot_xy(getTimes(states),
        toArray(map(map(states, fun), [&](Angle<double> x) {return x.degrees();})));
    plot.show();
  }

  void plotSpeed(const char *title, Array<BoatSim::FullState> states,
      std::function<Velocity<double>(BoatSim::FullState)> fun) {
    GnuplotExtra plot;
    plot.set_title(title);
    plot.set_style("lines");
    plot.plot_xy(getTimes(states),
        toArray(map(map(states, fun), [&](Velocity<double> x)
            {return x.knots();})));
    plot.show();
  }

  void plotTrajectory(Array<BoatSim::FullState> states) {
    GnuplotExtra plot;
    plot.set_title("Trajectory (meters)");
    plot.set_style("lines");
    plot.plot_xy(
        toArray(map(states, [&](BoatSim::FullState x) {return x.pos[0].meters();})),
        toArray(map(states, [&](BoatSim::FullState x) {return x.pos[1].meters();}))
    );
    plot.show();
  }
}

void BoatSim::makePlots(Array<BoatSim::FullState> states) {
  plotAngles("Rudder angle (degrees)", states, [](const BoatSim::FullState &x) {return x.rudderAngle;});
  plotAngles("Boat orientation (degrees)", states, [](const BoatSim::FullState &x) {return x.boatOrientation;});
  plotSpeed("Boat speed through water (knots)", states, [](const BoatSim::FullState &x) {return x.boatSpeedThroughWater;});
  plotTrajectory(states);
}



std::ostream &operator<<(std::ostream &s,
    const BoatSim::FullState &state) {
  s << EXPR_AND_VAL_AS_STRING(state.boatMotion) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.boatMotionThroughWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.boatOrientation) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.boatSpeedThroughWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.rudderAngle) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.time) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.trueCurrent) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.trueWind) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.windAngleWrtWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.windSpeedWrtWater) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.windWrtCurrent) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.pos[0]) << std::endl;
  s << EXPR_AND_VAL_AS_STRING(state.pos[1]) << std::endl;
  return s;
}



}
