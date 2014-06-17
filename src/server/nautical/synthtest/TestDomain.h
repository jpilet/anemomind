/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Defines a local domain in space and time, where a test case takes place.
 */

#ifndef TESTDOMAIN_H_
#define TESTDOMAIN_H_

#include <server/common/Span.h>
#include <server/nautical/GeographicReference.h>
#include <server/common/TimeStamp.h>

namespace sail {

class TestSpaceDomain {
 public:
  TestSpaceDomain() {}
  TestSpaceDomain(GeographicReference geor, LengthSpan xs, LengthSpan ys) : _geoRef(geor),
      _xSpan(xs), _ySpan(ys) {}
  bool validLocal(GeographicReference::ProjectedPosition pos) const {
    return _xSpan.contains(pos[0]) && _ySpan.contains(pos[1]);
  }

  GeographicReference::ProjectedPosition toLocal(const GeographicPosition<double> &pos) const;
  GeographicPosition<double> fromLocal(const GeographicReference::ProjectedPosition &pos) const;

  // Accessors
  const GeographicReference &geoRef() const {return _geoRef;}
  const LengthSpan &xSpan() const {return _xSpan;}
  const LengthSpan &ySpan() const {return _ySpan;}
 private:
   GeographicReference _geoRef;
   LengthSpan _xSpan, _ySpan;
};

class TestTimeDomain {
 public:
  TestTimeDomain() {}
  TestTimeDomain(TimeStamp tref, TimeSpan tspan) : _timeRef(tref), _timeSpan(tspan) {}
  bool validLocal(Duration<double> t) const {return _timeSpan.contains(t);}
  Duration<double> toLocal(TimeStamp ts) const;
  TimeStamp fromLocal(Duration<double> local) const;

  // Accessors
  TimeStamp timeRef() const {return _timeRef;}
  const TimeSpan &timeSpan() const {return _timeSpan;}
 private:
  // Time related
  TimeStamp _timeRef;
  TimeSpan _timeSpan;
};

class TestDomain {
 public:
  TestDomain() {}
  TestDomain(const TestSpaceDomain &spaceDomain,
               const TestTimeDomain &timeDomain);
  const TestSpaceDomain &space() const {return _spaceDomain;}
  const TestTimeDomain &time() const {return _timeDomain;}
 private:
  TestSpaceDomain _spaceDomain;
  TestTimeDomain _timeDomain;
};

} /* namespace sail */

#endif /* TESTDOMAIN_H_ */
