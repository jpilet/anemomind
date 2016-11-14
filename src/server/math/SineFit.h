/*
 * SineFit.h
 *
 *  Created on: 14 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_SINEFIT_H_
#define SERVER_COMMON_SINEFIT_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/Optional.h>

namespace sail {

struct SpacedAngles {
  Angle<double> offset;
  Angle<double> step;
  Angle<double> at(int n) const;
};

// f(x) = C*sin(omega*x + phi) + D
class Sine {
public:
  struct Sample {
    Angle<double> angle = 0.0_deg;
    double y = 0;
    double weight = 1.0;
    Sample() {}
    Sample(Angle<double> a, double b,
        double w = 1.0) : angle(a), y(b),
            weight(w) {}
  };

  Sine() : _C(0.0), _omega(0.0), _phi(0.0_deg), _D(0.0) {}
  Sine(double C, double omega, Angle<double> phi,
      double D) :
    _C(C), _omega(omega), _phi(phi), _D(D) {}
  double C() const {return _C;}
  double omega() const {return _omega;}
  Angle<double> phi() const {return _phi;}
  double D() const {return _D;}
  double operator()(Angle<double> x) const {
    return _C*sin(_omega*x + _phi) + _D;
  }
private:
  double _C, _omega, _D;
  Angle<double> _phi;
};

SpacedAngles minimize(const Sine &x);
SpacedAngles findExtrema(const Sine &x);

Optional<Sine> fit(double omega,
    const Array<Sine::Sample> &data);

} /* namespace sail */

#endif /* SERVER_COMMON_SINEFIT_H_ */
