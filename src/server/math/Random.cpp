/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/math/Random.h>
#include <server/common/logging.h>

namespace sail {

std::mt19937 makeRngForTests() {
  std::mt19937 rng(0);

  return rng;
}

}
