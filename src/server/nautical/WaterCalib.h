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
  static constexpr int thisParamCount = 5;

  // Don't set this value too low. If it is too low, we will not get sufficiently many inliers
  // to robustly determine the parameters. On the other hand, too high a value means an inaccurate
  // result because significant noise will influence the values.
  static Velocity<double> defaultSigma() {return Velocity<double>::knots(1.0);}

  // This parameter controls the convergence rate in the GemanMcClure robust cost function.
  // Choosing a higher value means slower but more robust convergence.
  static Velocity<double> defaultInitR() {return Velocity<double>::knots(1.0);}

  WaterCalib(const HorizontalMotionParam &param,
    Velocity<double> sigma = defaultSigma(),
    Velocity<double> initR = defaultInitR());

  int paramCount() const {
    return thisParamCount + _param.paramCount();
  }

  class Results {
   public:
    Results() : objfValue(std::numeric_limits<double>::infinity()) {}
    Results(Arrayb i, Arrayd p, Array<Nav> n, double ov, Arrayd re) :
      inliers(i),
      params(p),
      navs(n),
      objfValue(ov),
      rawErrors(re) {
        assert(n.size() == i.size());
        assert(re.size() == n.size());
      }

    double objfValue;
    Arrayb inliers;
    Arrayd params;
    Array<Nav> navs;
    Arrayd rawErrors;

    int inlierCount() const {
      return countTrue(inliers);
    }

    bool operator< (const Results &other) const {
      return objfValue < other.objfValue;
    }
  };

  Results optimize(Array<Nav> navs) const;
  Results optimize(Array<Nav> navs, Arrayd initParams) const;
  Results optimizeRandomInit(Array<Nav> navs) const;
  Results optimizeRandomInits(Array<Nav> navs, int iters) const;

  template <typename T>
  SpeedCalib<T> makeSpeedCalib(T *X) const {
    return SpeedCalib<T>(wcK(X), wcM(X), wcC(X), wcAlpha(X));
  }

  template <typename T>
  HorizontalMotion<T> calcBoatMotionRelativeToWater(const Nav &nav,
      T *params) const {
    return HorizontalMotion<T>::polar(nav.watSpeed().cast<T>(),
        nav.magHdg().cast<T>() + Angle<T>::radians(magOffset(params)));
  }

  const HorizontalMotionParam &horizontalMotionParam() const {
    return _param;
  }

  template <typename T> T *hmParams(T *x) const {return x + thisParamCount;}

  void makeWatSpeedCalibPlot(Arrayd params, Array<Nav> navs) const;

  template <typename T> static T unwrap(Velocity<T> x) {return x.metersPerSecond();}
  template <typename T> static T unwrap(Angle<T> x) {return x.radians();}
 private:

  Arrayd makeInitialParams() const;
  void initialize(double *outParams) const;
  void initializeRandom(double *outParams) const;
  Velocity<double> _sigma, _initR;
  const HorizontalMotionParam &_param;
  template <typename T> T &wcK(T *x) const {return x[0];}
  template <typename T> T &wcM(T *x) const {return x[1];}
  template <typename T> T &wcC(T *x) const {return x[2];}
  template <typename T> T &wcAlpha(T *x) const {return x[3];}
  template <typename T> T &magOffset(T *x) const {return x[4];}
};

}

#endif /* WATERCALIB_H_ */
