/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Conversion from/to Json for the Nav datatype.
 */

#include "NavJson.h"

namespace sail {
namespace json {

Poco::JSON::Object::Ptr encode(const Nav &nav) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
//  x->set("time-since-1970-s", nav.time().seconds());
//  x->set("lon-rad", nav.geographicPosition().lon().radians());
//  x->set("lat-rad", nav.geographicPosition().lat().radians());
//  x->set("alt-m", nav.geographicPosition().alt().meters());
//  x->set("awa-rad", nav.awa().radians());
//  x->set("aws-mps", nav.aws().metersPerSecond());
//  x->set("maghdg-rad", nav.magHdg().radians());
//  x->set("wat-speed-mps", nav.watSpeed().metersPerSecond());
//  x->set("gps-speed-mps", nav.gpsSpeed().metersPerSecond());
//  x->set("gps-bearing-rad", nav.gpsBearing().radians());
  return x;
}

Poco::JSON::Array encode(Array<Nav> navs) { // Perhaps write a template encodeArray<T> with T = Nav in this case...
  Poco::JSON::Array arr;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    arr.add(encode(navs[i]));
  }
  return arr;
}

}
} /* namespace sail */
