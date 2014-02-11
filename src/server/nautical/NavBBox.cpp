/*
 * NavBBox.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "NavBBox.h"

namespace sail {

NavBBox::NavBBox() {
  // TODO Auto-generated constructor stub

}

NavBBox::NavBBox(Array<Nav> navs) {
  int count = navs.size();
  Arrayd lon(count), lat(count), time(count);
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];
    lon[i] = nav.getLonRadians();
    lat[i] = nav.getLatRadians();
    time[i] = nav.getTimeSeconds();
  }
  _lon = CRange(lon, true);
  _lat = CRange(lat, false);
  _time = CRange(time, false);
}

bool NavBBox::intersects(const NavBBox &other) const {
  return _lon.intersects(other._lon) && _lat.intersects(other._lat) && _time.intersects(other._time);
}

Array<NavBBox> calcNavBBoxes(Array<Array<Nav> > navs) {
  return navs.map<NavBBox>([&] (Array<Nav> x) {
    return NavBBox(x);
  });
}

NavBBox::~NavBBox() {
  // TODO Auto-generated destructor stub
}

} /* namespace sail */
