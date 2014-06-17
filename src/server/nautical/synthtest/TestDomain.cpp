/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TestDomain.h"
#include <server/common/logging.h>

namespace sail {

Duration<double> TestTimeDomain::toLocal(TimeStamp ts) const {
  Duration<double> local = ts - _timeRef;
  CHECK(validLocal(local));
  return local;
}

TimeStamp TestTimeDomain::fromLocal(Duration<double> local) const {
  CHECK(validLocal(local));
  return _timeRef + local;
}

GeographicReference::ProjectedPosition TestSpaceDomain::toLocal(const GeographicPosition<double> &pos) const {
  GeographicReference::ProjectedPosition lpos = _geoRef.map(pos);
  CHECK(validLocal(lpos));
  return lpos;
}

GeographicPosition<double> TestSpaceDomain::fromLocal(const GeographicReference::ProjectedPosition &pos) const {
  CHECK(validLocal(pos));
  return _geoRef.unmap(pos);
}



TestDomain::TestDomain(const TestSpaceDomain &spaceDomain,
             const TestTimeDomain &timeDomain) :
             _spaceDomain(spaceDomain),
             _timeDomain(timeDomain) {}

bool TestDomain::operator== (const TestDomain &x) const {
  return _spaceDomain == x._spaceDomain && _timeDomain == x._timeDomain;
}

} /* namespace sail */
