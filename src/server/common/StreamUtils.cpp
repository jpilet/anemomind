/*
 * RedirectOstream.cpp
 *
 *  Created on: 21 Apr 2017
 *      Author: jonas
 */

#include "StreamUtils.h"
#include <iostream>

namespace sail {

RedirectOstream::RedirectOstream(
    std::ostream* target,
    std::streambuf* dst) :
    _target(target), _backup(target->rdbuf()) {
  _target->rdbuf(dst);
}

RedirectOstream::~RedirectOstream() {
  _target->rdbuf(_backup);
}


} /* namespace sail */
