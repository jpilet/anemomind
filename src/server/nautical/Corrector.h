/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATIONMODEL_H_
#define CALIBRATIONMODEL_H_

#include <memory>
#include <server/nautical/SpeedCalib.h>
#include <server/common/Array.h>
#include <server/common/ExpLine.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>
#include <server/common/ToDouble.h>

namespace sail {

#pragma pack(push, 1)
  template <typename T>
  class AngleCorrector {
   public:
    T value;

    AngleCorrector() : value(0) {}
    Angle<T> correct(Angle<T> raw) const {
      return raw + Angle<T>::degrees(T(value));
    }
  };

  template <typename T>
  class SpeedCorrector {
   public:
    T k, m, c, alpha;

    SpeedCorrector() :
      k(SpeedCalib<T>::initKParam()),
      m(SpeedCalib<T>::initMParam()),
      c(SpeedCalib<T>::initCParam()),
      alpha(SpeedCalib<T>::initAlphaParam()) {}


    SpeedCalib<T> make() const {
      return SpeedCalib<T>(k, m, c, alpha);
    }

    Velocity<T> correct(Velocity<T> raw) const {
      return make().eval(raw);
    }
  };

  template <typename T>
  class DriftAngleCorrector {
   public:
    T amp, coef;

    DriftAngleCorrector() :
     amp(0), coef(-2) {}

    Angle<T> correct(const CalibratedNav<T> &c) const {
      T awa0rads = c.calibAwa().normalizedAt0().radians();

      // For awa angles closer to 0 than 90 degrees,
      // scale by sinus of that angle. Otherwise, just use 0.
      T awaFactor = amp*(2.0*std::abs(ToDouble(awa0rads)) < M_PI? T(sin(2.0*awa0rads)) : T(0));

      // Scale it in a way that decays exponentially as
      // aws increases. The decay is controlled by params[1].
      T awsFactor = exp(-expline(coef)*c.calibAws().metersPerSecond());

      return Angle<T>::radians(awaFactor*awsFactor);
    }
  };

  template <typename T>
  class Corrector {
   public:
    AngleCorrector<T> awa, magHdg;
    SpeedCorrector<T> aws, watSpeed;
    DriftAngleCorrector<T> driftAngle;

    static int paramCount() {
      return sizeof(Corrector<T>)/sizeof(T);
    }

    template <typename InstrumentAbstraction>
    CalibratedNav<T> correct(const InstrumentAbstraction &x) const {
      CalibratedNav<T> c(x);
      c.calibWatSpeed.set(watSpeed.correct(c.rawWatSpeed()));
      c.calibAws.set(aws.correct(c.rawAws()));
      c.calibAwa.set(awa.correct(c.rawAwa()));
      c.boatOrientation.set(magHdg.correct(c.rawMagHdg()));
      c.driftAngle.set(driftAngle.correct(c));
      fillRemainingValues(&c);
      return c;
    }


    // This method just wraps the data of this
    // object inside an Array<T> without copying
    // anything.
    // To allocate a new
    // reference counted array that will persist
    // even after this object has been destroyed,
    // call toArray().dup().
    Array<T> toArray() const {
      return Array<T>(paramCount(), (T *)(this));
    }

    // Just to hide the pointer cast.
    static Corrector<T> *fromPtr(T *ptr) {
      return reinterpret_cast<Corrector<T> *>(ptr);
    }

    static Corrector<T> *fromArray(Array<T> arr) {
      assert(arr.size() == paramCount());
      return fromPtr(arr.ptr());
    }
   private:
    // Fill in the remainig values after the raw measurements and driftAngle
    // have been corrected for.
    void fillRemainingValues(CalibratedNav<T> *dst) const {
      // Compute the true wind
      dst->directionApparentWindBlowsTo.set(dst->calibAwa() + dst->boatOrientation()
          + Angle<T>::degrees(T(180)));
      dst->apparentWind.set(HorizontalMotion<T>::polar(dst->calibAws(),
          dst->directionApparentWindBlowsTo()));
      dst->trueWind.set(dst->apparentWind() + dst->gpsMotion());

      // Compute the true current
      dst->boatMotionThroughWater.set(HorizontalMotion<T>::polar(
          dst->calibWatSpeed(), dst->driftAngle() + dst->boatOrientation()));
      dst->trueCurrent.set(dst->gpsMotion() - dst->boatMotionThroughWater());
    }
  };

#pragma pack(pop)
}



#endif /* CALIBRATIONMODEL_H_ */
