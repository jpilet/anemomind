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
class LocalStateAssign {
 public:
  // Standard penalty for hard constraints. Maybe it makes
  // more sense here to use a large number than infinity, in case
  // it is impossible to satisfy all constraints...
  static constexpr double HardPenalty = 1.0e9;

  /*
   * Methods to override
   */
  virtual int begin() const = 0;
  virtual int end() const = 0;
  virtual int getStateCount() = 0;
  virtual double getStateCost(int stateIndex, int timeIndex) = 0;
  virtual double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) = 0;

  /*
   * Other methods
   */
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

  virtual ~LocalStateAssign() {}
};

/*
 * This class takes a standard StateAssign object 'ref'
 * and augments it with hints of type LocalStateAssign.
 *
 * Every hint adds a local cost to the costs originally computed
 * by 'ref'.
 *
 * This object is a StateAssign object itself and calling
 * its method 'solve' will solve the problem specified by 'ref'
 * together with the hints. If no hints are provided,
 * its result is the same as that of calling solve() on 'ref'.
 */
class HintedStateAssign : public StateAssign {
 public:
  typedef std::shared_ptr<LocalStateAssign> LocalStateAssignPtr;
  typedef SpanOverlap<LocalStateAssignPtr, int> Overlap;

  HintedStateAssign(std::shared_ptr<StateAssign> ref,
      Array<LocalStateAssignPtr> hints);

  double getStateCost(int stateIndex, int timeIndex);
  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);

  int getStateCount() {
    return _ref->getStateCount();
  }

  int getLength() {
    return _ref->getLength();
  }
  Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
    return _ref->getPrecedingStates(stateIndex, timeIndex);
  }
 private:
  std::shared_ptr<StateAssign> _ref;

  Arrayi _stateTable, _transitionTable;
  Array<Overlap> _stateOverlaps, _transitionOverlaps;
};

}

#endif /* HINTEDSTATEASSIGN_H_ */
