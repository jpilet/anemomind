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
  std::shared_ptr<LocalStateAssign> makeNonEmpty(TimeStamp ts, MDArray2b table, Array<Nav> navs) {
    if (!table.empty()) {
      Array<Nav>::Iterator start = navs.begin();

      // Compute lower and upper bounds
      Array<Nav>::Iterator lower = std::lower_bound(navs.begin(), navs.end(), Nav(ts));
      Array<Nav>::Iterator upper = std::upper_bound(navs.begin(), navs.end(), Nav(ts));
      int timeIndex = lower - navs.begin();
      int dif = upper - lower;
      if (dif >= 0) {
        return std::shared_ptr<LocalStateAssign>(new TransitionHint(table, timeIndex));
      }
    }
    return std::shared_ptr<LocalStateAssign>();
  }

  std::shared_ptr<LocalStateAssign> hintNotFound() {
    return std::shared_ptr<LocalStateAssign>();
  }
}

std::shared_ptr<LocalStateAssign> TransitionHint::make(const UserHint &hint, Array<Nav> navs, const Grammar &dst) {
  switch (hint.type()) {
  case UserHint::RACE_START:
    return makeNonEmpty(hint.time(), dst.startOfRaceTransitions(), navs);
  case UserHint::RACE_END:
    return makeNonEmpty(hint.time(), dst.endOfRaceTransitions(), navs);
  default:
    return hintNotFound();
  };
  return hintNotFound();
}

}
