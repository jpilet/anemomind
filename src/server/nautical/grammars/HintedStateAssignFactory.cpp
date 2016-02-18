/*
 *  Created on: 2014-07-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HintedStateAssignFactory.h"
#include <server/nautical/grammars/TransitionHint.h>
#include <server/common/logging.h>
#include <sstream>

namespace sail {

HintedStateAssign makeHintedStateAssign(const Grammar &g, std::shared_ptr<StateAssign> ref,
  Array<UserHint> hints, NavCollection navs) {
  ArrayBuilder<std::shared_ptr<LocalStateAssign> > hintsLSA;
  for (auto h : hints) {
    std::shared_ptr<LocalStateAssign> t = TransitionHint::make(h, navs, g);
    if (bool(t)) {
      hintsLSA.add(t);
    } else {
      std::stringstream ss;
      ss << "Hint not supported: " << int(h.type()) << " at time " << h.time().toString();
      LOG(FATAL) << ss.str();
    }
  }
  return HintedStateAssign(ref, hintsLSA.get());
}


} /* namespace mmm */
