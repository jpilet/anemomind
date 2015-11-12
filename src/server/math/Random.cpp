/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/math/Random.h>
#include <server/common/logging.h>
#include <iostream>

namespace sail {

std::mt19937 makeRngForTests() {
  std::mt19937 rng(0);
  // Check that the RNG always works the same when we do tests:
  CHECK(rng() == 2357136044);
  return rng;
}

}
