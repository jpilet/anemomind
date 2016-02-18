/*
 *  Created on: 2014-07-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TransitionHint.h"
#include <server/common/logging.h>
#include <algorithm>
#include <server/nautical/grammars/Grammar.h>
#include <server/common/string.h>
#include <iostream>
#include <cassert>

namespace sail {


double TransitionHint::getTransitionCost(int fromStateIndex,
    int toStateIndex, int fromTimeIndex) {
    if (fromTimeIndex == _timeIndex) {
      return (_validTransitions(fromStateIndex, toStateIndex)? 0 : HardPenalty);
    }
    LOG(FATAL) << "If there are no bugs in the program, we should never get here.";
    return HardPenalty;
}

namespace {
  std::shared_ptr<LocalStateAssign> makeNonEmpty(TimeStamp ts, MDArray2b table, NavCollection navs) {
    if (!table.empty()) {
      auto start = navs.begin();
      Nav x(ts);
      auto upper = std::upper_bound(start, navs.end(), x);
      int timeIndex = (upper - start) - 1;
      if (upper != navs.end() && timeIndex >= 0) {
        return std::shared_ptr<LocalStateAssign>(new TransitionHint(table, timeIndex));
      }
    }
    return std::shared_ptr<LocalStateAssign>();
  }

  std::shared_ptr<LocalStateAssign> hintNotFound() {
    return std::shared_ptr<LocalStateAssign>();
  }
}

std::shared_ptr<LocalStateAssign> TransitionHint::make(const UserHint &hint, NavCollection navs, const Grammar &dst) {
  switch (hint.type()) {
  case UserHint::RACE_START:
    return makeNonEmpty(hint.time(), dst.startOfRaceTransitions(), navs);
  case UserHint::RACE_END:
    return makeNonEmpty(hint.time(), dst.endOfRaceTransitions(), navs);
  };
  return hintNotFound();
}

}
