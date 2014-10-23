/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATIONMODEL_H_
#define CALIBRATIONMODEL_H_

#include <server/nautical/SpeedCalib.h>

namespace sail {

/*
 * Use SI units whenever
 * the PhysicalQuantity type is not used.
 */

template <typename T>
class Corrector {
 public:
  virtual int paramCount() const = 0;
  virtual void initialize(T *dst) const = 0;
  T *initializeAndNext(T *dst) const {
    initialize(dst);
    return dst + paramCount();
  }
  virtual T correct(T *calibParameters, T x) const = 0;
  virtual ~Corrector() {}
};

template <typename T>
class OffsetCorrector : public Corrector<T> {
 public:
  int paramCount() const {return 1;}
  void initialize(T *dst) const {dst[0] = 0;}
  T correct(T *calibParameters, T x) const {
    return x + calibParameters[0];
  }
};

template <typename T>
class SpeedCorrector : public Corrector<T> {
 public:
  int paramCount() const {return 4;}
  void initialize(T *dst) const {
    dst[0] = SpeedCalib<T>::initKParam();
    dst[1] = SpeedCalib<T>::initMParam();
    dst[2] = SpeedCalib<T>::initCParam();
    dst[3] = SpeedCalib<T>::initAlphaParam();
  }
  T correct(T *calibParameters, T x) const {
    SpeedCalib<T> calib(calibParameters[0],
        calibParameters[1], calibParameters[2],
        calibParameters[3]);
    return calib.eval(Velocity<T>::metersPerSeconds(x)).metersPerSecond(x);
  }
 private:
};

/*
 * Given calibrated values of AWA and AWS, computes a
 * correction angle.
 */
template <typename T>
class DriftAngle {
 public:
  virtual int paramCount() const {return 2;}
  virtual void initialize(T *dst) const {
    dst[0] = 0;   // Maximum value of the
    dst[1] = -2;  // Slope
  }
  virtual T correct(T *params, T x) const {
    return params[0]*exp(-expline(params[1])*x);
  }

  virtual ~DriftAngle() {}
 private:
};

/*
 * A class that groups together
 * all the models used to calibrate the various
 * values.
 */
template <typename T>
class CorrectorSet {
 public:
  virtual const Corrector<T> &magneticHeadingCorrector() const = 0;
  virtual const Corrector<T> &waterSpeedCorrector() const = 0;
  virtual const Corrector<T> &awaCorrector() const = 0;
  virtual const Corrector<T> &awsCorrector() const = 0;
  virtual const DriftAngle<T> &driftAngle() const = 0;

  T *magneticHeadingParams(T *x) const {return x + 0;}
  T *waterSpeedParams(T *x) const {return x +
      magneticHeadingCorrector().paramCount();}
  T *awaParams(T *x) const {
    return waterSpeedParams(x) + waterSpeedCorrector().paramCount();
  }
  T *awsParams(T *x) const {
    return awaParams(x) + awaCorrector().paramCount();
  }
  T *driftAngleParams(T *x) const {
    return awsParams(x) + awsCorrector().paramCount();
  }

  int paramCount() const {
    return magneticHeadingCorrector().paramCount()
        + waterSpeedCorrector().paramCount()
        + awaCorrector().paramCount()
        + awsCorrector().paramCount()
        + driftAngle().paramCount();
  }

  void initialize(T *dst) const {
     driftAngle().initialize(
         awsCorrector().initializeAndNext(
             awaCorrector().initializeAndNext(
                 waterSpeedCorrector().initializeAndNext(
                     magneticHeadingCorrector().initializeAndNext(dst)))));
  }

 virtual ~CorrectorSet() {}
};

template <typename T>
void evaluateTrueWindAndCurrent(
  const CorrectorSet<T> &correctors,
  T *parameters,
  HorizontalMotion<T> gpsMotion,
  Angle<T> rawMagneticHeading,
  Velocity<T> rawWaterSpeed,
  Angle<T> rawAwa,
  Velocity<T> rawAws,
  HorizontalMotion<T> *outTrueWind,
  HorizontalMotion<T> *outTrueCurrent,
  ) {
  // Initial corrections
  Angle<T> awa = Angle<T>::radians(correctors.awaCorrector().eval(
      correctors.awaParams(parameters), rawAwa.radians()));
  Angle<T> boatOrientation = Angle<T>::radians(correctors.magneticHeadingCorrector().eval(
      correctors.magneticHeadingParams(parameters), rawMagneticHeading.radians()));
  Velocity<T> aws = Velocity<T>::metersPerSeconds(correctors.awaCorrector().eval(
      correctors.awsParams(parameters), rawAws.metersPerSecond()));
  Velocity<T> waterSpeed = Velocity<T>::metersPerSeconds(correctors.waterSpeedCorrector().eval(
      correctors.waterSpeedParams(parameters), rawWaterSpeed.metersPerSecond()));

  // Compute the true wind
  Angle<T> apparentWindAngleWrtEarth = awa + boatOrientation + Angle<T>::degrees(T(180));
  HorizontalMotion<T> apparentWind = HorizontalMotion<T>::polar(aws,
      apparentWindAngleWrtEarth);
  *outTrueWind = apparentWind + gpsMotion;

  // Compute the true current

}

}

#endif /* CALIBRATIONMODEL_H_ */
