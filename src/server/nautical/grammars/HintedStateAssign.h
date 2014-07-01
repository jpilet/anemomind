/*
 *  Created on: 2014-07-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HINTEDSTATEASSIGN_H_
#define HINTEDSTATEASSIGN_H_

#include <server/math/hmm/StateAssign.h>
#include <server/common/SpanOverlap.h>

namespace sail {

/*
 * Represents a state assign for which the costs are only defined
 * for a span of time indices.
 * */
class LocalStateAssign : public StateAssign {
 public:
  virtual int begin() const = 0;
  virtual int end() const = 0;

  int beginStateIndex() const {
    return begin();
  }

  int endStateIndex() const {
    return end() - 1;
  }

  bool validTimeIndex(int index) const {
    return begin() <= index && index < end();
  }

  double getSafeStateCost(int stateIndex, int timeIndex) {
    assert(validTimeIndex(timeIndex));
    return getStateCost(stateIndex, timeIndex);
  }

  double getSafeTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
    assert(validTimeIndex(fromTimeIndex + 0));
    assert(validTimeIndex(fromTimeIndex + 1));
    return getTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);
  }
};

class HintedStateAssign : public StateAssign {
 public:
  typedef std::shared_ptr<LocalStateAssign> LocalStateAssignPtr;
  typedef SpanOverlap<LocalStateAssignPtr, int> Overlap;

  HintedStateAssign(Array<LocalStateAssignPtr> refAndHints);

  double getStateCost(int stateIndex, int timeIndex);
  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);

  int getStateCount() {
    return _ref->getStateCount();
  }

  int getLength() {
    return _ref->getLength();
  }
  Arrayi getPrecedingStates(int stateIndex, int timeIndex);
 private:
  LocalStateAssignPtr _ref;


  Arrayi _stateTable, _transitionTable;
  Array<Overlap> _stateOverlaps, _transitionOverlaps;
};

}

#endif /* HINTEDSTATEASSIGN_H_ */
