/*
 *  Created on: 10 avr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavIndexer.h"
#include <server/common/string.h>
#include <iostream>

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

namespace {

  int64_t reverse(int64_t x) {
    int64_t y;
    uint8_t *xb = (uint8_t *)(&x);
    uint8_t *yb = (uint8_t *)(&y);
    constexpr int count = sizeof(x);
    for (int i = 0; i < count; i++) {
      yb[i] = xb[count-1-i];
    }
    return y;
  }

  std::string int64ToHexLittleEndian(int64_t x) {
    return bytesToHex(sizeof(x), (uint8_t *)(&x));
  }

  std::string int64ToHex(int64_t x) {
    if (isBigEndian()) {
      return int64ToHexLittleEndian(x);
    }
    return int64ToHexLittleEndian(reverse(x));
  }
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
