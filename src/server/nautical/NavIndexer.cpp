/*
 *  Created on: 10 avr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavIndexer.h"
#include <server/common/string.h>
#include <iostream>
#include <server/common/endianess.h>

namespace sail {

Nav NavIndexer::make(const Nav &src) {
  Nav dst = src;
  assert(!dst.isIndexed());
  dst.setId(makeId(src));
  dst.setBoatId(boatId());
  assert(dst.isIndexed());
  return dst;
}


BoatTimeNavIndexer::BoatTimeNavIndexer(Nav::Id boatId8hexDigits) :
  _boatId(boatId8hexDigits) {
  assert(isHexString(_boatId, 8));
}

BoatTimeNavIndexer BoatTimeNavIndexer::makeTestIndexer() {
  return BoatTimeNavIndexer(debuggingBoatId());
}

Nav::Id BoatTimeNavIndexer::makeId(const Nav &src) {
  int64_t time = src.time().toMilliSecondsSince1970();
  assert(sizeof(time) == 8); // 8 bytes => 2*8 = 16 hex digits
  std::string s = makeIdSub(time);
  assert(s.length() == 24);
  return s;
}

std::string BoatTimeNavIndexer::TimeGen::make(int64_t x) {
  if (x == _lastTime) {
    _counter++;
  } else {
    _lastTime = x;
    _counter = 0;
  }
  // Add a few milliseconds to make the id unique. I know it's a bit hacky but this was the format we decided...
  return int64ToHex(x + _counter);
}

Nav::Id BoatTimeNavIndexer::makeIdSub(int64_t time) {
  return _boatId + _tgen.make(time);
}



} /* namespace sail */
