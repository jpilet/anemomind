/*
 * NavBBox.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef NAVBBOX_H_
#define NAVBBOX_H_

#include "Nav.h"
#include <server/common/ContinuousRange.h>
#include <server/common/Span.h>

namespace sail {

// Nav Bounding box. Used to check if to recordings are related.
class NavBBox {
 public:
  NavBBox();
  NavBBox(Array<Nav> nav);
  virtual ~NavBBox();

  bool intersects(const NavBBox &other) const;
 private:
  PeriodicSpan _lon, _lat;
  Span<TimeStamp> _time;
};

Array<NavBBox> calcNavBBoxes(Array<Array<Nav> > navs);

} /* namespace sail */

#endif /* NAVBBOX_H_ */
