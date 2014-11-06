/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATIONMODEL_H_
#define CALIBRATIONMODEL_H_

#include <memory>
#include <cmath>
#include <server/nautical/SpeedCalib.h>
#include <server/common/Array.h>
#include <server/common/ToDouble.h>
#include <server/common/ExpLine.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>
#include <server/common/SharedPtrUtils.h>

namespace sail {


template <typename T>
class Corrector {
 public:
  typedef std::shared_ptr<Corrector<T> > Ptr;
  virtual void apply(T *parameters, CalibratedNav<T> *dst) const = 0;
  virtual int paramCount() const = 0;
  virtual void initialize(double *params) const = 0;
  virtual std::shared_ptr<Corrector<T> > toDouble() const = 0;
  virtual ~Corrector() {}
};


// TODO: WHENEVER USING THIS MACRO:
//       Make sure that 'type' corresponds to the class name.
#define MAKE_TO_DOUBLE(type) \
    Corrector<double>::Ptr toDouble() const {\
      static type<double> x; \
      return makeSharedPtrToStack(x); \
      }


/*
 *  Default method to correct angles.
 *
 *  We can override the methods of this class
 *  to correct the angle in a custom way.
 */
template <typename T>
class AngleCorrector : public Corrector<T> {
 public:
  int paramCount() const {return 1;}
  void initialize(double *dst) const {
    dst[0] = 0;
  }
 protected:
  T correct(T *parameters, Angle<T> x) const {
    return x + Angle<T>::degrees(parameters[0]);
  }
};

template <typename T>
class AwaCorrector : public AngleCorrector<T> {
 public:
  void apply(T *parameters, CalibratedNav<T> *dst) const {
    dst->calibAwa.set(correct(parameters, dst->rawAwa.get()));
  }
  MAKE_TO_DOUBLE(AwaCorrector)
};

template <typename T>
class MagHdgCorrector : public AngleCorrector<T> {
 public:
  void apply(T *parameters, CalibratedNav<T> *dst) const {
    dst->boatOrientation.set(correct(parameters, dst->rawMagHdg.get()));
  }
  MAKE_TO_DOUBLE(MagHdgCorrector)
};



template <typename T>
class SpeedCorrector {
 public:
  int paramCount() const {return 4;}
  void initialize(double *dst) const {
    dst[0] = SpeedCalib<double>::initKParam();
    dst[1] = SpeedCalib<double>::initMParam();
    dst[2] = SpeedCalib<double>::initCParam();
    dst[3] = SpeedCalib<double>::initAlphaParam();
  }
protected:
  Velocity<T> correct(T *calibParameters, Velocity<T> x) const {
    SpeedCalib<T> calib(calibParameters[0],
        calibParameters[1], calibParameters[2],
        calibParameters[3]);
    return calib.eval(x);
  }
};

template <typename T>
class WatSpeedCorrector : public SpeedCorrector<T> {
 public:
  void apply(T *parameters, CalibratedNav<T> *dst) const {
    dst->calibWatSpeed.set(correct(parameters, dst->rawWatSpeed.get()));
  }
  MAKE_TO_DOUBLE(WatSpeedCorrector)
 private:
};

template <typename T>
class AwsCorrector : public SpeedCorrector<T> {
 public:
  void apply(T *parameters, CalibratedNav<T> *dst) const {
    dst->calibAwsSpeed.set(correct(parameters, dst->rawAwsSpeed.get()));
  }
  MAKE_TO_DOUBLE(AwsCorrector)
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
  int paramCount() const {return 2;}
  void initialize(double *dst) const {
    dst[0] = 0;
    dst[1] = -2;
  }

  void apply(T *params, CalibratedNav<T> *dst) const {
    T awa0rads = dst->calibAwa.get().normalizedAt0().radians();

    // For awa angles closer to 0 than 90 degrees,
    // scale by sinus of that angle. Otherwise, just use 0.
    T awaFactor = params[0]*(2.0*std::abs(ToDouble(awa0rads)) < M_PI? T(sin(2.0*awa0rads)) : T(0));

    // Scale it in a way that decays exponentially as
    // aws increases. The decay is controlled by params[1].
    T awsFactor = exp(-expline(params[1])*dst->calibAws.metersPerSecond());

    dst->driftAngle.set(Angle<T>::radians(awaFactor*awsFactor));
  }

  MAKE_TO_DOUBLE(DriftAngle)
};



/*
 * In the constructor, this class calibrates
 * most relevant values that we would like to know.
 */
template <typename T>
class CorrectorSet : public Corrector<T> {
 public:
  /*
   * It is the responsibility of the user of
   * this class to provide the correctors ordered,
   * so that a corrector A whose output is used by
   * a corrector B precedes it in the array, that is:
   *
   *   correctors = (... , A, ..., B, ...)
   *
   * but not
   *
   *   correctors = (..., B, ..., A, ...)
   *
   * Failure to do so will result in a runtime error,
   * when trying to access an undefined value in the
   * DefinedValue class.
   */

  // Typedef to easier avoid intricate compilation errors :-)
  typedef typename Corrector<T>::Ptr CorrectorPtr;

  CorrectorSet(Array<CorrectorPtr> correctors) :
    _correctors(correctors) {
    _paramCount = 0;
    for (auto c: correctors) {
      _paramCount += c->paramCount();
    }
  }
  int paramCount() const {
    return _paramCount;
  }

  void apply(T *parameters, CalibratedNav<T> *dst) const {
    int offset = 0;
    for (auto c: _correctors) {
      c->apply(parameters + offset, dst);
      offset += c->paramCount();
    }
  }

  void initialize(double *params) const {
    int offset = 0;
    for (auto c: _correctors) {
      c->initialize(params);
      offset += c->paramCount();

    }
  }

  std::shared_ptr<Corrector<double> > toDouble() const {
    typedef Corrector<double>::Ptr CorrectorPtrd;
    int count = _correctors.size();
    Array<CorrectorPtrd> dcorrs(count);
    for (int i = 0; i < count; i++) {
      dcorrs[i] = _correctors[i]->toDouble();
    }
    return Corrector<double>::Ptr(new CorrectorSet<double>(dcorrs));
  }
 private:
  int _paramCount;
  Array<CorrectorPtr> _correctors;
};


}

#endif /* CALIBRATIONMODEL_H_ */
