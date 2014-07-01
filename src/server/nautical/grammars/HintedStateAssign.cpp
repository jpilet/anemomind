/*
 *  Created on: 2014-07-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HintedStateAssign.h"
#include <server/common/Span.h>
#include <server/common/logging.h>

namespace sail {

HintedStateAssign::HintedStateAssign(Array<LocalStateAssignPtr> hints) {
  Array<Spani> spans = hints.map<Spani>([=] (const LocalStateAssignPtr &hint) {
    return Spani(hint->begin(), hint->end());
  });
  _overlaps = Overlap::compute(spans, hints);
  CHECK(_overlaps.first().span().minv() == 0);
}


} /* namespace mmm */
