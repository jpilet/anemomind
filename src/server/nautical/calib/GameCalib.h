/*
 * GameCalib.h
 *
 *  Created on: 30 Dec 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_GAMECALIB_H_
#define SERVER_NAUTICAL_CALIB_GAMECALIB_H_

#include <server/nautical/calib/CalibDataChunk.h>
#include <server/common/DOMUtils.h>
#include <adolc/adouble.h>

namespace sail {
namespace GameCalib {

class Sensor {
public:
  virtual void initialize(double *dst) = 0;
  virtual int parameterCount() const = 0;
  virtual std::string explain(double *parameter) = 0;
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
  std::string explain(double *parameter) override;
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

  std::string explain(double *parameters) override;
private:
  Array<BasisFunction> _basis;
};

enum class PlayerType {
  /*
   * Fitness, over all sensors
   *   - Minimizing fitness to wind speed observations
   *   - Minimizing fitness to wind angle observations
   *      (in speed domain, weighted by the underlying estimated wind
   *       speed)
   * W.r.t, for all sensors
   *   - Wind speed sensor parameters
   *   - Wind angle sensor parameters
   */
  Wind,

  /*
   * Fitness, over all sensors
   *   -
   */
  Current, //
  IMU,     // Unknowns: Calibration parameters for every IMU
  Leeway   // Unknown: Leeway parameter for the boat
};

inline Velocity<adouble> unitVelocity(Velocity<adouble>) {
  return Velocity<adouble>::metersPerSecond(1.0);
}

inline Velocity<adouble> identityVelocity(Velocity<adouble> x) {
  return x;
}

struct Settings {
  Settings();
  Duration<double> currentSamplingPeriod = 1.0_minutes;
  Duration<double> windSamplingPeriod = 1.0_minutes;

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
