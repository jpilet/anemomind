/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/common/numerics.h>
#include <server/nautical/AbsoluteOrientation.h>

namespace sail {

bool isFinite(const AbsoluteOrientation &orientation) {
  return isFinite(orientation.heading) &&
      isFinite(orientation.pitch) && isFinite(orientation.roll);
}

}


