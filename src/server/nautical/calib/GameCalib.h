/*
 * GameCalib.h
 *
 *  Created on: 30 Dec 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_GAMECALIB_H_
#define SERVER_NAUTICAL_CALIB_GAMECALIB_H_

#include <server/nautical/calib/CalibDataChunk.h>
#include <server/math/nonlinear/GameSolver.h>
#include <server/common/DOMUtils.h>

namespace sail {
namespace GameCalib {

class Sensor {
public:
  virtual void initialize(double *dst) = 0;
  virtual int parameterCount() const = 0;
  virtual ~Sensor() {}
};

class VelocitySensor : public Sensor {
public:
  typedef std::shared_ptr<VelocitySensor> Ptr;

  virtual Velocity<adouble> correct(
      adouble *parameters,
      const Velocity<adouble> &x) = 0;
  virtual Velocity<adouble> corrupt(
      adouble *parameters,
      const Velocity<adouble> &x) = 0;
};

class AngleSensor : public Sensor {
public:
  typedef std::shared_ptr<AngleSensor> Ptr;

  virtual Angle<adouble> correct(
      adouble *parameters,
      Angle<adouble> x) = 0;
  virtual Angle<adouble> corrupt(
      adouble *parameters,
      Angle<adouble> x) = 0;
};

class BasicAngleSensor : public AngleSensor {
public:
  void initialize(double *dst);
  int parameterCount() const;
  Angle<adouble> correct(
      adouble *parameters,
      Angle<adouble> x);
  Angle<adouble> corrupt(
      adouble *parameters,
      Angle<adouble> x);
};

class LinearVelocitySensor : public VelocitySensor {
public:
  typedef std::function<Velocity<adouble>(Velocity<adouble>)> BasisFunction;
  LinearVelocitySensor(const Array<BasisFunction> &basis);

  virtual void initialize(double *dst);
  virtual int parameterCount() const;
  virtual Velocity<adouble> correct(
      adouble *parameters,
      const Velocity<adouble> &x);
  virtual Velocity<adouble> corrupt(
      adouble *parameters,
      const Velocity<adouble> &x);
private:
  Array<BasisFunction> _basis;
};

enum class PlayerType {
  Wind,
  Current,
  IMU,
  Leeway
};

inline Velocity<adouble> unitVelocity(Velocity<adouble>) {
  return Velocity<adouble>::metersPerSecond(1.0);
}

inline Velocity<adouble> identityVelocity(Velocity<adouble> x) {
  return x;
}

struct Settings {
  Duration<double> currentSamplingPeriod = 1.0_minutes;
  Duration<double> windSamplingPeriod = 1.0_minutes;

  GameSolver::Settings solverSettings;
  std::set<PlayerType> playerTypesToConsider = {PlayerType::Current};

  AngleSensor::Ptr magHeadingSensor
    = AngleSensor::Ptr(new BasicAngleSensor());

  VelocitySensor::Ptr watSpeedSensor
    = VelocitySensor::Ptr(new LinearVelocitySensor({
      &unitVelocity,
      &identityVelocity
  }));
};

void optimize(
    const Array<CalibDataChunk> &chunks,
    const Settings &settings,
    DOM::Node *dst);

}
}

#endif /* SERVER_NAUTICAL_CALIB_GAMECALIB_H_ */
