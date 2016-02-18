/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cmath>
#include <server/common/MeanAndVar.h>
#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/PhysicalQuantityIO.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <server/nautical/Nav.h>

#ifndef SERVER_NAUTICAL_FLOWERRORS_H_
#define SERVER_NAUTICAL_FLOWERRORS_H_

namespace sail {


class FlowErrors {
 public:
  template <typename T>
  class Error {
   public:
    Error() : _unit(T::zero()) {}
    Error(const MeanAndVar &mv, T unit) :
      _mv(mv), _unit(unit) {}

    bool undefined() const {
      return _mv.empty();
    }

    T mean() const {
      return (_mv.empty()? NAN : _mv.mean())*_unit;
    }
    T rms() const {
      return (_mv.empty()? NAN : _mv.rms())*_unit;
    }

    Error operator+ (const Error &other) const {
      assert(_unit == other._unit);
      return Error(_mv + other._mv, _unit);
    }

    int count() const {
      return _mv.count();
    }
   private:
    MeanAndVar _mv;
    T _unit;
  };

  FlowErrors() {}
  FlowErrors(Array<HorizontalMotion<double> > trueMotion,
            Array<HorizontalMotion<double> > estimatedMotion);

  Error<Velocity<double> > norm() const {
    return _normError;
  }

  Error<Velocity<double> > magnitude() const {
    return _magnitudeError;
  }

  Error<Angle<double> > angle() const {
    return _angleError;
  }

  FlowErrors operator+ (const FlowErrors &other) const {
    return FlowErrors(_normError + other._normError,
        _magnitudeError + other._magnitudeError,
        _angleError + other._angleError);
  }

  int count() const {
    return _normError.count();
  }
 private:
  FlowErrors(const Error<Velocity<double> > &ne,
      const Error<Velocity<double> > &me,
      const Error<Angle<double> > &ae) :
      _normError(ne), _angleError(ae),
      _magnitudeError(me) {}
  Error<Velocity<double> > _normError, _magnitudeError;
  Error<Angle<double> > _angleError;
  static double nanIfEmpty(const MeanAndVar &mv, double x) {
    return (mv.empty()? NAN : x);
  }
};

template <typename T>
std::ostream &operator<< (std::ostream &s,
    const FlowErrors::Error<T> &e) {
  s << "Error(mean = " << e.mean() << ", rms = " << e.rms() << ")";
  return s;
}

std::ostream &operator<< (std::ostream &s, const FlowErrors &e);

struct WindCurrentErrors {
  FlowErrors wind, current;
  int count;
};

std::ostream &operator<< (std::ostream &s, const WindCurrentErrors &e);

WindCurrentErrors compareCorrectors(
    const CorrectorFunction &a,
    const CorrectorFunction &b,
    NavCollection navs);

}

#endif /* SERVER_NAUTICAL_FLOWERRORS_H_ */
