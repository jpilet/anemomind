/*
 *  Created on: 2014-07-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HintedStateAssign.h"
#include <server/common/Span.h>
#include <server/common/logging.h>
#include <server/transducers/Transducer.h>

namespace sail {

namespace {
  int calcTIndex(int x) {
    return 2*x + 1;
  }

  Spani makeTSpan(const HintedStateAssign::LocalStateAssignPtr &x) {
    return Spani(calcTIndex(x->beginStateIndex()) - 1,
                 calcTIndex(x->endStateIndex()) - 1);
  }

  void computeOverlapsAndTable(int len, Array<Spani> spans,
      Array<HintedStateAssign::LocalStateAssignPtr> hints,
    Array<HintedStateAssign::Overlap> *overlaps, Arrayi *outTable) {
    typedef HintedStateAssign::Overlap Overlap;
    *overlaps = Overlap::compute(spans, hints);
    *outTable = Arrayi::fill(len, -1);
    int n = overlaps->size();
    for (int i = 0; i < n; i++) {
      Overlap x = (*overlaps)[i];
      outTable->slice(x.span().minv(), x.span().maxv()).setTo(i);
    }
  }
}

HintedStateAssign::HintedStateAssign(std::shared_ptr<StateAssign> ref,
    Array<LocalStateAssignPtr> hints) : _ref(ref) {
  if (hints.hasData()) {
    CHECK(hints.same<int>([&](const LocalStateAssignPtr &h) {return h->getStateCount();}));
    CHECK(hints[0]->getStateCount() == _ref->getStateCount());
  }
  auto stateSpans = transduce(
      hints,
      trMap([=] (const LocalStateAssignPtr &hint) {
        return Spani(hint->begin(), hint->end());
      }),
      IntoArray<Spani>());

  auto transitionSpans = transduce(
      hints,
      trMap([=] (const LocalStateAssignPtr &hint) {
        return makeTSpan(hint);
      }),
      IntoArray<Spani>());

  int len = ref->getLength();
  computeOverlapsAndTable(len, stateSpans, hints, &_stateOverlaps, &_stateTable);
  computeOverlapsAndTable(calcTIndex(len), transitionSpans, hints, &_transitionOverlaps, &_transitionTable);
}

double HintedStateAssign::getStateCost(int stateIndex, int timeIndex) {
  double cost = _ref->getStateCost(stateIndex, timeIndex);
  int i = _stateTable[timeIndex];
  if (i == -1) {
    return cost;
  }

  const Array<LocalStateAssignPtr> &X = _stateOverlaps[i].objects();
  for (auto x : X) {
    cost += x->getSafeStateCost(stateIndex, timeIndex);
  }
  return cost;
}

double HintedStateAssign::getTransitionCost(int fromStateIndex,
    int toStateIndex, int fromTimeIndex) {
  double cost = _ref->getTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);

  int i = _transitionTable[calcTIndex(fromTimeIndex)];
  if (i == -1) {
    return cost;
  }

  const Array<LocalStateAssignPtr> &X =
      _transitionOverlaps[i].objects();
  for (auto x : X) {
    cost += x->getSafeTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);
  }
  return cost;
}


} /* namespace mmm */
