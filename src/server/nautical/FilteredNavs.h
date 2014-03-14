/*
 *  Created on: 2014-03-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Holds filtered signals from several Navs in a race
 */

#ifndef FILTEREDNAVS_H_
#define FILTEREDNAVS_H_

#include <server/common/Array.h>
#include <server/nautical/Nav.h>
#include <server/math/Grid.h>

namespace sail {

Arrayb identifyReliableValues(int stateCount, double transitionCost, Arrayd values, Span span = Span());
Arrayb identifyReliableAws(Array<Velocity<double> > aws);
Arrayb identifyReliableAwa(Array<Angle<double> > awa);

class FilteredNavs {
 public:
  FilteredNavs();
  virtual ~FilteredNavs();

 private:
  LineStrip _strip;
  Arrayd _cosAwa, _sinAwa;
};

} /* namespace sail */

#endif /* FILTEREDNAVS_H_ */
