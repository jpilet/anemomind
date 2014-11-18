/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatSim.h"
#include <cassert>

namespace sail {


BoatSimulator::BoatSimulator(
    FlowFun windFun,
    FlowFun currentFun,
    BoatCharacteristics::Ptr ch,
    Array<TwaDirective> twaSpans) :
        _windFun(windFun),
        _currentFun(currentFun),
        _ch(ch), _twaSpans(twaSpans) {
  _indexer = ProportionateIndexer(twaSpans.map<double>([&](TwaDirective d) {
    return d.duration.seconds();
  }));
}

void BoatSimulator::eval(double *Xin, double *Fout, double *Jout) {
  assert(Jout == nullptr);
  BoatSimulationState &state = *((BoatSimulationState *)Xin);
  BoatSimulationState &deriv = *((BoatSimulationState *)Fout);

  Angle<double> rudderAngle = Angle<double>::radians(state.rudderAngleRadians);
  Length<double> x = Length<double>::meters(state.boatXMeters);
  Length<double> y = Length<double>::meters(state.boatYMeters);
  Duration<double> time = Duration<double>::seconds(state.timeSeconds);
  Angle<double> boatOrientation = Angle<double>::radians(state.boatOrientationRadians);

  Velocity<double> boatSpeedThroughWater = Velocity<double>::metersPerSecond(state.boatSpeedThroughWaterMPS);

  HorizontalMotion<double> trueWind = _windFun(x, y, time);
  HorizontalMotion<double> trueCurrent = _currentFun(x, y, time);

  // Since a polar table will assume no current, it makes sense
  // to compute the "true" wind in a coordinate system attached to
  // the local water surface.
  HorizontalMotion<double> windWrtCurrent = trueWind - trueCurrent;

  Angle<double> twaWater = (windWrtCurrent.angle() + Angle<double>::radians(M_PI)) - boatOrientation;
  Velocity<double> twsWater = windWrtCurrent.norm();

  HorizontalMotion<double> boatMotionThroughWater =
      HorizontalMotion<double>::polar(boatSpeedThroughWater, boatOrientation);

  HorizontalMotion<double> boatMotion = trueCurrent + boatMotionThroughWater;

  deriv.boatSpeedThroughWaterMPS =
      _ch->targetSpeedGain()*(
          _ch->targetSpeed(twaWater, twsWater).metersPerSecond() - state.boatSpeedThroughWaterMPS)
        - std::abs(_ch->rudderResistanceCoef()*sin(rudderAngle))*sqr(state.boatSpeedThroughWaterMPS);
  deriv.boatXMeters = boatMotion[0].metersPerSecond();
  deriv.boatYMeters = boatMotion[1].metersPerSecond();
  deriv.boatOrientationRadians = -boatSpeedThroughWater.metersPerSecond()*sin(rudderAngle)
      /(_ch->keelRudderDistance().meters());
  deriv.rudderAngleRadians = _ch->rudderCorrectionCoef()*(getTargetTwa(time) - twaWater).radians();
  deriv.timeSeconds = 1.0;

}

}
