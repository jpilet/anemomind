/*
 *  Created on: 2014-07-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HintedStateAssign.h"
#include <server/common/Span.h>
#include <server/common/logging.h>

namespace sail {

namespace {
  int calcTIndex(int x) {
    return 2*x + 1;
  }

  Spani makeTSpan(const HintedStateAssign::LocalStateAssignPtr &x) {
    return Spani(calcTIndex(x->beginStateIndex()) - 1,
                 calcTIndex(x->endStateIndex()) + 1);
  }

  void computeOverlapsAndTable(Array<Spani> spans,
      Array<HintedStateAssign::LocalStateAssignPtr> hints,
    Array<HintedStateAssign::Overlap> *overlaps, Arrayi *outTable) {
    typedef HintedStateAssign::Overlap Overlap;
    *overlaps = Overlap::compute(spans, hints);
    *outTable = Arrayi(overlaps->last().span().maxv());
    int n = overlaps->size();
    for (int i = 0; i < n; i++) {
      Overlap x = (*overlaps)[i];
      outTable->slice(x.span().minv(), x.span().maxv()).setTo(i);
    }
  }
}

HintedStateAssign::HintedStateAssign(std::shared_ptr<StateAssign> ref,
    Array<LocalStateAssignPtr> hints) {
  CHECK(hints.same<int>([&](const LocalStateAssignPtr &h) {return h->getStateCount();}));
  Array<Spani> stateSpans = hints.map<Spani>([=] (const LocalStateAssignPtr &hint) {
    return Spani(hint->begin(), hint->end());
  });

  Array<Spani> transitionSpans = hints.map<Spani>([=] (const LocalStateAssignPtr &hint) {
    return makeTSpan(hint);
  });

  computeOverlapsAndTable(stateSpans, hints, &_stateOverlaps, &_stateTable);
  computeOverlapsAndTable(transitionSpans, hints, &_transitionOverlaps, &_transitionTable);
}

double HintedStateAssign::getStateCost(int stateIndex, int timeIndex) {
  double cost = _ref->getStateCost(stateIndex, timeIndex);
  const Array<LocalStateAssignPtr> &X = _stateOverlaps[_stateTable[timeIndex]].objects();
  for (auto x : X) {
    cost += x->getSafeStateCost(stateIndex, timeIndex);
  }
  return cost;
}

double HintedStateAssign::getTransitionCost(int fromStateIndex,
    int toStateIndex, int fromTimeIndex) {
  double cost = _ref->getTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);
  const Array<LocalStateAssignPtr> &X =
      _transitionOverlaps[_transitionTable[calcTIndex(fromTimeIndex)]].objects();
  for (auto x : X) {
    cost += x->getSafeTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);
  }
  return cost;
}


} /* namespace mmm */
