/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATIONMODEL_H_
#define CALIBRATIONMODEL_H_

#include <memory>
#include <cmath>
#include <server/nautical/SpeedCalib.h>
#include <server/common/Uniform.h>
#include <server/common/Array.h>
#include <server/common/ToDouble.h>
#include <server/common/ExpLine.h>

namespace sail {


/*
 *  Default method to correct angles.
 *
 *  We can override the methods of this class
 *  to correct the angle in a custom way.
 */
template <typename T>
class AngleCorrector {
 public:
  virtual int paramCount() const {return 1;}
  virtual void initialize(double *dst, double randomness) const {
    Uniform rng(-randomness, randomness);
    dst[0] = rng.gen() + 0.2*rng.gen();
  }
  virtual Angle<T> correct(T *calibParameters, Angle<T> x) const {
    return Angle<T>::radians(x.radians() + calibParameters[0]);
  }
  virtual ~AngleCorrector() {}
};

/*
 *  Default method to correct speeds.
 *
 *  We can override the methods to
 *  implement a custom way of correcting
 *  the speed.
 *
 */
template <typename T>
class SpeedCorrector {
 public:
  virtual int paramCount() const {return 4;}
  virtual void initialize(double *dst, double randomness) const {
    Uniform rng(-randomness, randomness);
    dst[0] = SpeedCalib<double>::initKParam() + 0.2*rng.gen();
    dst[1] = SpeedCalib<double>::initMParam() + 3.0*rng.gen();
    dst[2] = SpeedCalib<double>::initCParam() + 3.0*rng.gen();
    dst[3] = SpeedCalib<double>::initAlphaParam() + rng.gen();
  }

  virtual void initializeRandom(double *dst) {

  }

  virtual Velocity<T> correct(T *calibParameters, Velocity<T> x) const {
    SpeedCalib<T> calib(calibParameters[0],
        calibParameters[1], calibParameters[2],
        calibParameters[3]);
    return calib.eval(x);
  }
  virtual ~SpeedCorrector() {}
 private:
};

/*
 * Given calibrated values of AWA and AWS, computes a
 * correction angle.
 *
 * We can create our own class inheriting from
 * this class in order to provide our own drift model.
 */
template <typename T>
class DriftAngle {
 public:
  virtual int paramCount() const {return 2;}
  virtual void initialize(double *dst, double randomness) const {
    Uniform rng(-randomness, randomness);
    dst[0] = 0 + rng.gen();   // Maximum value of the
    dst[1] = -2 + rng.gen();  // Slope
  }
  virtual Angle<T> eval(T *params, Angle<T> awa, Velocity<T> aws) const {
    T awa0rads = awa.normalizedAt0().radians();

    // For awa angles closer to 0 than 90 degrees,
    // scale by sinus of that angle. Otherwise, just use 0.
    T awaFactor = params[0]*(2.0*std::abs(ToDouble(awa0rads)) < M_PI? T(sin(awa0rads)) : T(0));

    // Scale it in a way that decays exponentially as
    // aws increases. The decay is controlled by params[1].
    T awsFactor = exp(-expline(params[1])*aws.metersPerSecond());

    return Angle<T>::radians(awaFactor*awsFactor);
  }

  virtual ~DriftAngle() {}
 private:
};

/*
 * A class that groups together
 * all the models used to calibrate the various
 * values.
 *
 * We can inherit from this class and provide our
 * own models to calibrate the various values.
 */
template <typename T>
class CorrectorSet {
 public:
  typedef CorrectorSet<T> ThisType;
  typedef std::shared_ptr<ThisType> Ptr;

  virtual const AngleCorrector<T> &magneticHeadingCorrector() const = 0;
  virtual const SpeedCorrector<T> &waterSpeedCorrector() const = 0;
  virtual const AngleCorrector<T> &awaCorrector() const = 0;
  virtual const SpeedCorrector<T> &awsCorrector() const = 0;
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



  void initialize(double *dst, double randomness) const {
    double *x = dst;
    initializeAndStep(magneticHeadingCorrector(), randomness, &x);
    initializeAndStep(waterSpeedCorrector(), randomness, &x);
    initializeAndStep(awaCorrector(), randomness, &x);
    initializeAndStep(awsCorrector(), randomness, &x);
    initializeAndStep(driftAngle(), randomness, &x);
    assert(dst + paramCount() == x);
  }

  Arrayd makeInitialParams(double randomness) const {
    Arrayd dst(paramCount());
    initialize(dst.ptr(), randomness);
    return dst;
  }

  virtual ~CorrectorSet() {}

  virtual std::shared_ptr<CorrectorSet<double> > toDouble() const = 0;
 private:
   template <typename Corr>
   static void initializeAndStep(const Corr &c, double randomness, double **dst) {
     c.initialize(*dst, randomness);
     (*dst) += c.paramCount();
   }
};

template <typename T>
class DefaultCorrectorSet : public CorrectorSet<T> {
 public:
  const AngleCorrector<T> &magneticHeadingCorrector() const {
    return _angleCorrector;
  }

  const SpeedCorrector<T> &waterSpeedCorrector() const {
    return _speedCorrector;
  }

  const AngleCorrector<T> &awaCorrector() const {
    return _angleCorrector;
  }

  const SpeedCorrector<T> &awsCorrector() const {
    return _speedCorrector;
  }

  const DriftAngle<T> &driftAngle() const {
    return _driftAngle;
  }

  std::shared_ptr<CorrectorSet<double> > toDouble() const {
    return std::shared_ptr<CorrectorSet<double> >(
        new DefaultCorrectorSet<double>());
  }
 private:
  AngleCorrector<T> _angleCorrector;
  SpeedCorrector<T> _speedCorrector;
  DriftAngle<T> _driftAngle;
};

/*
 * In the constructor, this class calibrates
 * most relevant values that we would like to know.
 */
template <typename T>
class CalibratedValues {
 public:
  Angle<T> awa;
  Angle<T> boatOrientation;
  Velocity<T> aws;
  Velocity<T> waterSpeed;
  Angle<T> driftAngle;
  Angle<T> apparentWindAngleWrtEarth;
  HorizontalMotion<T> gpsMotion;
  HorizontalMotion<T> apparentWind;
  HorizontalMotion<T> trueWind;
  HorizontalMotion<T> trueCurrent;
  HorizontalMotion<T> boatMotionThroughWater;

  CalibratedValues() {}
  CalibratedValues(const CorrectorSet<T> &correctors,
      T *parameters,
      HorizontalMotion<T> gpsMotion_,
      Angle<T> rawMagneticHeading,
      Velocity<T> rawWaterSpeed,
      Angle<T> rawAwa,
      Velocity<T> rawAws) {
    gpsMotion = gpsMotion_; // <-- nothing to calibrate, already accurate.
    awa = correctors.awaCorrector().correct(
        correctors.awaParams(parameters), rawAwa);
    boatOrientation = correctors.magneticHeadingCorrector().correct(
        correctors.magneticHeadingParams(parameters), rawMagneticHeading);
    aws = correctors.awsCorrector().correct(
        correctors.awsParams(parameters), rawAws);
    waterSpeed = correctors.waterSpeedCorrector().correct(
        correctors.waterSpeedParams(parameters), rawWaterSpeed);
    driftAngle = correctors.driftAngle().eval(
              correctors.driftAngleParams(parameters), awa, aws);

    // Compute the true wind
    apparentWindAngleWrtEarth = awa + boatOrientation + Angle<T>::degrees(T(180));
    apparentWind = HorizontalMotion<T>::polar(aws,
        apparentWindAngleWrtEarth);
    trueWind = apparentWind + gpsMotion;

    // Compute the true current
    boatMotionThroughWater = HorizontalMotion<T>::polar(
        waterSpeed, driftAngle + boatOrientation);
    trueCurrent = gpsMotion - boatMotionThroughWater;
  }
};


}

#endif /* CALIBRATIONMODEL_H_ */
