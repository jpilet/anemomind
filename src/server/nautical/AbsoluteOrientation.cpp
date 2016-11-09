/*
 * AbsoluteOrientation.cpp
 *
 *  Created on: Apr 1, 2016
 *      Author: jonas
 */

#include <server/nautical/AbsoluteOrientation.h>
#include <server/common/numerics.h>

namespace sail {

bool isFinite(const AbsoluteOrientation &x) {
  return isFinite(x.heading) && isFinite(x.pitch) && isFinite(x.roll);
}

}




