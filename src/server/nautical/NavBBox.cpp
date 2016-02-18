/*
 * NavBBox.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "NavBBox.h"
#include <server/common/Functional.h>

namespace sail {

NavBBox::NavBBox() {
  // TODO Auto-generated constructor stub

}

NavBBox::NavBBox(NavCollection navs) {
  int count = navs.size();
  Arrayd lon(count), lat(count);
  Array<TimeStamp> time(count);
  for (int i = 0; i < count; i++) {
    Nav nav = navs[i];
    const GeographicPosition<double> &pos = nav.geographicPosition();

    lon[i] = pos.lon().radians();
    lat[i] = pos.lat().radians();
    time[i] = nav.time();
  }
  _lon = PSpan(lon);
  _lat = PSpan(lat);
  _time = Span<TimeStamp>(time);
}

bool NavBBox::intersects(const NavBBox &other) const {
  return _lon.intersects(other._lon) && _lat.intersects(other._lat) && _time.intersects(other._time);
}

Array<NavBBox> calcNavBBoxes(Array<NavCollection> navs) {
  return toArray(map(navs, [&] (NavCollection x) {
    return NavBBox(x);
  }));
}

NavBBox::~NavBBox() {
  // TODO Auto-generated destructor stub
}

} /* namespace sail */
