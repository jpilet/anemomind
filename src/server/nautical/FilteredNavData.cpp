/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FilteredNavData.h"
#include <algorithm>
#include <server/common/logging.h>
#include <server/math/CleanNumArray.h>

namespace sail {

namespace {
  bool sameBoat(Array<Nav> navs) {
    Nav::Id boatId = navs[0].boatId();
    int count = navs.size();
    for (int i = 1; i < count; i++) {
      if (boatId != navs[i].boatId()) {
        return false;
      }
    }
    return true;
  }

  Array<Duration<double> > getTimes(Array<Nav> navs, TimeStamp offset) {
    return navs.map<Duration<double> >([](const Nav &nav) {
      return nav.time() - offset;
    });
  }
}

FilteredNavData::FilteredNavData(Array<Nav> navs, double lambda) {
  if (navs.hasData()) {
    std::sort(navs.begin(), navs.end());
    if (sameBoat(navs)) {
      _timeOffset = navs[0].time();
      Array<Duration<double> > times = getTimes(navs, _timeOffset);

    } else {
      LOG(WARNING) << "Mixed boat ids";
    }
  } else {
    LOG(WARNING) << "No navs";
  }
}

}
