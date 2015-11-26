/*
 *  Created on: 2014-06-19
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "UserHint.h"
#include <assert.h>
#include <map>
#include <server/common/Array.h>
#include <server/common/Functional.h>

namespace sail {

namespace {
  Array<std::string> makeTypeStringTable() {
    Array<std::string> table(UserHint::NUM_TYPES);
    table[UserHint::UNDEFINED] = "undefined";
    table[UserHint::RACE_START] = "race-start";
    table[UserHint::RACE_END] = "race-end";

    assert(all(sail::map(table, [&](const std::string &x) {
      return !x.empty();
    })));
    return table;
  }

  const std::string &mapTypeToString(UserHint::HintType type) {
    static Array<std::string> table = makeTypeStringTable();
    return table[type];
  }

  typedef std::map<std::string, UserHint::HintType> HintMap;
  HintMap makeTypeFromStringMap() {
    HintMap x;
    for (int i = 0; i < UserHint::NUM_TYPES; i++) {
      UserHint::HintType type = UserHint::HintType(i);
      x[mapTypeToString(type)] = type;
    }
    return x;
  }

  UserHint::HintType typeFromString(const std::string &str) {
    static HintMap map = makeTypeFromStringMap();
    return map[str];
  }
}

UserHint::UserHint(std::string type, TimeStamp time_) :
    _type(typeFromString(type)), _time(time_) {}

std::string UserHint::typeAsString() const {
  return mapTypeToString(_type);
}


} /* namespace sail */
