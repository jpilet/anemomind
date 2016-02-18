/*
 * NavBBox.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef NAVBBOX_H_
#define NAVBBOX_H_

#include "Nav.h"
#include <server/common/PeriodicSpan.h>
#include <server/common/Span.h>

namespace sail {

// Nav Bounding box. Used to check if to recordings are related.
class NavBBox {
 public:
  NavBBox();
  NavBBox(NavCollection nav);
  virtual ~NavBBox();

  bool intersects(const NavBBox &other) const;
 private:
  PeriodicSpan _lon, _lat;
  Span<TimeStamp> _time;
};

Array<NavBBox> calcNavBBoxes(Array<NavCollection> navs);

} /* namespace sail */

#endif /* NAVBBOX_H_ */
