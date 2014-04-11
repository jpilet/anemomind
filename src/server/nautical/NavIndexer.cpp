/*
 *  Created on: 10 avr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavIndexer.h"
#include <server/common/string.h>

namespace sail {

Nav NavIndexer::make(const Nav &src) {
  Nav dst = src;
  assert(!dst.isIndexed());
  dst.setId(makeId(src));
  dst.setBoatId(boatId());
  assert(dst.isIndexed());
  return dst;
}


BoatTimeNavIndexer::BoatTimeNavIndexer(Nav::Id boatId8hexDigits,
    Nav::Id highestId16hexDigits) :
  _boatId(boatId8hexDigits),
  _highestId(highestId16hexDigits) {
  assert(isHexString(_boatId, 8));

  if (!highestId16hexDigits.empty()) {
    assert(isHexString(_highestId, 16));
    assert(_boatId == _highestId.substr(0, 8));
  }
}

BoatTimeNavIndexer BoatTimeNavIndexer::makeTestIndexer() {
  return BoatTimeNavIndexer(debuggingBoatId(), "");
}

Nav::Id BoatTimeNavIndexer::makeId(const Nav &src) {
  int64_t time = src.time().toMilliSecondsSince1970();
  assert(sizeof(time) == 8); // 8 bytes => 2*8 = 16 hex digits

  Nav::Id s = makeIdSub(time);
  if (!_highestId.empty()) {
    while (_highestId >= s) { // <-- Ensure that we generate a unique id.
      time++;
      s = makeIdSub(time);
    }
  }

  _highestId = s;

  assert(s.length() == 24);
  return s;
}

Nav::Id BoatTimeNavIndexer::makeIdSub(int64_t time) {
  return _boatId + bytesToHex(sizeof(time), (uint8_t *)(&time));
}



} /* namespace sail */
