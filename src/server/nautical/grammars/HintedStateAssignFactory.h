/*
 *  Created on: 2014-07-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HINTEDSTATEASSIGNFACTORY_H_
#define HINTEDSTATEASSIGNFACTORY_H_

#include <server/nautical/Nav.h>
#include <server/nautical/grammars/HintedStateAssign.h>
#include <server/nautical/grammars/Grammar.h>

namespace sail {


  HintedStateAssign makeHintedStateAssign(const Grammar &g, std::shared_ptr<StateAssign> ref,
      Array<UserHint> hints, Array<Nav> navs);

} /* namespace mmm */

#endif /* HINTEDSTATEASSIGNFACTORY_H_ */
