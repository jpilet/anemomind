/*
 * FrequencyLimiter.cpp
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#include "FrequencyLimiter.h"

namespace sail {

bool FrequencyLimiter::accept(TimeStamp t) {
  if (_last.undefined()) {
    _last = t;
    return true;
  }
   auto d = t - _last;
   if (_minPeriod <= d) {
     _last = t;
     return true;
   }
   return false;
 }

} /* namespace sail */
