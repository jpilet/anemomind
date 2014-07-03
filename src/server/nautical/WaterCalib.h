/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef WATERCALIB_H_
#define WATERCALIB_H_

#include <server/common/Array.h>
#include <server/nautical/Nav.h>
#include <server/nautical/SpeedCalib.h>
#include <server/nautical/HorizontalMotionParam.h>

namespace sail {


class WaterCalib {
 public:
  static Velocity<double> defaultSigma() {return Velocity<double>::knots(0.2);}
  static Velocity<double> defaultInitR() {return Velocity<double>::knots(1.0);}

  WaterCalib(const HorizontalMotionParam &param,
    Velocity<double> sigma = defaultSigma(), Velocity<double> initR = defaultInitR());


  int paramCount() const {
    return 5 + _param.paramCount();
  }

  class Results {
   public:
    Results() {}
    Results(Arrayb i, Arrayd p) : inliers(i), params(p) {}

    Arrayb inliers;
    Arrayd params;

    int inlierCount() const {
      return countTrue(inliers);
    }
  };

  Results optimize(Array<Nav> navs) const;

  template <typename T>
  SpeedCalib<T> makeSpeedCalib(T *X) const {
    return SpeedCalib<T>(wcK(X), wcM(X), wcC(X), wcAlpha(X));
  }

  template <typename T> T unwrap(Velocity<T> x) const {return x.metersPerSecond();}
  template <typename T> Velocity<T> wrapVelocity(T x) const {return Velocity<T>::metersPerSecond(x);}

  template <typename T> T unwrap(Angle<T> x) const {return x.radians();}
  template <typename T> Angle<T> wrapAngle(T x) const {return Angle<T>::radians(x);}

  template <typename T>
  HorizontalMotion<T> calcBoatMotionRelativeToWater(const Nav &nav,
      const SpeedCalib<T> &sc, T *params) const {
    return HorizontalMotion<T>::polar(nav.watSpeed().cast<T>(),
        nav.magHdg().cast<T>() + wrapAngle(magOffset(params)));
  }

  const HorizontalMotionParam &horizontalMotionParam() const {
    return _param;
  }

  template <typename T> T *hmotionParams(T *x) const {return x + 5;}
 private:
  Arrayd makeInitialParams() const;
  void initialize(double *outParams) const;
  Velocity<double> _sigma, _initR;
  const HorizontalMotionParam &_param;
  template <typename T> T &wcK(T *x) const {return x[0];}
  template <typename T> T &wcM(T *x) const {return x[1];}
  template <typename T> T &wcC(T *x) const {return x[2];}
  template <typename T> T &wcAlpha(T *x) const {return x[3];}
  template <typename T> T &magOffset(T *x) const {return x[4];}

  //SpeedCalib(T k, T m, T c, T alpha) :
};

}

#endif /* WATERCALIB_H_ */
