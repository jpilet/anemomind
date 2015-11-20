/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATIONMODEL_H_
#define CALIBRATIONMODEL_H_

#include <memory>
#include "SpeedCalib.h"

#ifdef ON_SERVER
#include <server/common/Array.h>
#include <server/common/string.h>
#include <iostream>
#endif

#include <server/common/ExpLine.h>
#include "../CalibratedNav/CalibratedNav.h"
#include <server/common/Functional.h>

namespace sail {

#pragma pack(push, 1) // Since PhysicalQuantity is currently not packed, store the numbers raw.

  template <typename T>
  class AngleCorrector {
   public:
    AngleCorrector(Angle<double> x) : value(x.degrees()) {}

    T value;

    AngleCorrector() : value(0) {}
    Angle<T> correct(Angle<T> raw) const {
      auto x = raw + Angle<T>::degrees(T(value));
      assert(!genericIsNan(x));
      return x;
    }
  };

  template <typename T>
  class SpeedCorrector {
   public:
    SpeedCorrector(T k_, T m_, T c_, T alpha_) : k(k_), c(c_), m(m_), alpha(alpha_) {}
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
      auto x = make().eval(raw);
      assert(!genericIsNan(x));
      return x;
    }
  };


  template <typename T>
  class DriftAngleCorrector {
   public:
    T amp, coef;

    DriftAngleCorrector(T amp_, T coef_) : amp(amp_), coef(coef_) {}

    DriftAngleCorrector() :
     amp(0), coef(-2) {}

    Angle<T> correct(const CalibratedNav<T> &c) const {
      T awa0rads = c.calibAwa().normalizedAt0().radians();
      assert(!genericIsNan(awa0rads));


      //I am not sure we need 'ToDouble': bool upwind = 2.0*std::abs(ToDouble(awa0rads)) < M_PI;

      bool upwind =  (-M_PI/2 < awa0rads) && (awa0rads < M_PI/2);

      // For awa angles closer to 0 than 90 degrees,
      // scale by sinus of that angle. Otherwise, just use 0.
      T awaFactor = amp*(upwind? T(sin(2.0*awa0rads)) : T(0));
      assert(!genericIsNan(awaFactor));

      auto caws = c.calibAws().metersPerSecond();
      if (caws < T(0)) {
        caws = T(0);
      }

      // Scale it in a way that decays exponentially as
      // aws increases. The decay is controlled by params[1].
      T awsFactor = exp(-expline(coef)*caws);
      assert(!genericIsNan(awsFactor));

      auto v = Angle<T>::radians(awaFactor*awsFactor);
      assert(!genericIsNan(v));
      return v;
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


    #ifdef ON_SERVER
    Array<T> toArray() const {
      return Array<T>(paramCount(), (T *)(this));
    }

    static Corrector<T> *fromArray(Array<T> arr) {
      assert(arr.size() == paramCount());
      return fromPtr(arr.ptr());
    }
    #endif

    // Just to hide the pointer cast.
    static Corrector<T> *fromPtr(T *ptr) {
      return reinterpret_cast<Corrector<T> *>(ptr);
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
      dst->trueWindOverGround.set(dst->apparentWind() + dst->gpsMotion());

      // Compute the true current
      dst->boatMotionOverWater.set(HorizontalMotion<T>::polar(
          dst->calibWatSpeed(), dst->driftAngle() + dst->boatOrientation()));
      dst->trueCurrentOverGround.set(dst->gpsMotion() - dst->boatMotionOverWater());
    }
  };
#pragma pack(pop)

template <typename T>
std::ostream &operator<<(std::ostream &s, AngleCorrector<T> corr) {
  s << "AngleCorrector(value=" << corr.value << ")";
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, SpeedCorrector<T> corr) {
  s << "SpeedCorrector(k=" << corr.k << ", m=" << corr.m << ", c=" << corr.c << ", alpha=" << corr.alpha << ")";
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, DriftAngleCorrector<T> corr) {
  s << "DriftAngleCorrector(amp=" << corr.amp << ", " << corr.coef << ")";
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, Corrector<T> corr) {
  s << "Corrector:\n";
  s << "  awa: " << corr.awa << "\n";
  s << "  magHdg: " << corr.magHdg << "\n";
  s << "  aws: " << corr.aws << "\n";
  s << "  watSpeed: " << corr.watSpeed << "\n";
  s << "  driftAngle: " << corr.driftAngle << "\n";
  return s;
}

// Adapts it so that we can use it in benchmarks
class CorrectorObject : public CorrectorFunction {
 public:
  CorrectorObject(Corrector<double> c) : _c(c) {}

  Array<CalibratedNav<double> > operator()(const Array<Nav> &navs) const {
    return toArray(map([&](const Nav &x) {return _c.correct(x);}, navs));
  }

  std::string toString() const {
    std::stringstream ss;
    ss << _c;
    return ss.str();
  }
 private:
  Corrector<double> _c;
};


}
#endif /* CALIBRATIONMODEL_H_ */
