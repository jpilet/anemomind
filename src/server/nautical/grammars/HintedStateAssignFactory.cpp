/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HintedStateAssignFactory.h"
#include <server/nautical/grammars/TransitionHint.h>
#include <server/common/logging.h>

namespace sail {

HintedStateAssign HintedStateAssignFactory::make(const Grammar &g, std::shared_ptr<StateAssign> ref,
  Array<UserHint> hints, Array<Nav> navs) {
  ArrayBuilder<std::shared_ptr<LocalStateAssign> > hintsLSA;
  for (auto h : hints) {
    std::shared_ptr<LocalStateAssign> t = TransitionHint::make(h, navs, g);
    if (bool(t)) {
      hintsLSA.add(t);
    } else {
      LOG(FATAL) << "Hint not supported.";
    }
  }
  return HintedStateAssign(ref, hintsLSA.get());
}


} /* namespace mmm */
