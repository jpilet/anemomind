/*
 *  Created on: 2014-07-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TransitionHint.h"
#include <server/common/logging.h>

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

    }
    return std::shared_ptr<LocalStateAssign>();
  }
}

std::shared_ptr<LocalStateAssign> TransitionHint::make(const UserHint &hint, Array<Nav> navs, const Grammar &dst) {
  switch (hint.type()) {
  case RACE_START:
  case RACE_END:
  default:
    return std::shared_ptr<LocalStateAssign>();
  };
}

}
