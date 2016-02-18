/*
 *  Created on: 2014-07-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TRANSITIONHINT_H_
#define TRANSITIONHINT_H_

#include <server/nautical/grammars/HintedStateAssign.h>
#include <server/nautical/grammars/UserHint.h>
#include <server/nautical/Nav.h>

namespace sail {


class Grammar;

class TransitionHint : public LocalStateAssign {
 public:
  // The transition takes place from state at 'timeIndex' to state at 'timeIndex+1'
  TransitionHint(MDArray2b validTransitions, int timeIndex) :
    _validTransitions(validTransitions), _timeIndex(timeIndex) {
    assert(_validTransitions.rows() == _validTransitions.cols());
  }

  int begin() const {
    return _timeIndex;
  }

  int end() const {
    return _timeIndex+2;
  }
  int getStateCount() {
    return _validTransitions.rows();
  }

  double getStateCost(int stateIndex, int timeIndex) {
    return 0;
  }

  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);

  static std::shared_ptr<LocalStateAssign> make(const UserHint &hint,
      NavCollection navs, const Grammar &dst);
  private:
   MDArray2b _validTransitions;
   int _timeIndex;
};

}

#endif /* TRANSITIONHINT_H_ */
