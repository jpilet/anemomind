/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "StateAssignOps.h"
#include <server/common/logging.h>

namespace sail {

namespace {
  class StateAssignSum : public StateAssign {
   public:
    StateAssignSum(std::shared_ptr<StateAssign> A,
         std::shared_ptr<StateAssign> B);

    double getStateCost(int stateIndex, int timeIndex) {
      return _A->getStateCost(stateIndex, timeIndex) + _B->getStateCost(stateIndex, timeIndex);
    }

    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      return _A->getTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex)
          + _B->getTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);
    }

    int getStateCount() {
      return _A->getStateCount();
    }

    int getLength() {
      return _A->getLength();
    }

    /* This one is a bit tricky: What should we do here?
     * Maybe intersection, since a state being missing in this list
     * might mean the cost to go from it is infinite, and adding
     * an infinite cost to any other cost yields an infinite cost.
     * On the other hand, that would be complicated.
     *
     * What about simply assuming that either _A or _B is responsible
     * for supplying a list of predecessors, and that the other object
     * should return an empty list? This is easier to implement and I
     * don't think we need anything more sophisticated.
     */
    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
      Arrayi Astates = _A->getPrecedingStates(stateIndex, timeIndex);
      Arrayi Bstates = _B->getPrecedingStates(stateIndex, timeIndex);
      if (Astates.empty()) {
        return Bstates;
      } else if (Bstates.empty()) {
        return Astates;
      } else {
        LOG(FATAL) << "Either Astates or Bstates should be empty, that is, undefined. Either _A or _B is responsible for providing a list of predecessors";
        return Arrayi();
      }
    }
   private:
    std::shared_ptr<StateAssign> _A;
    std::shared_ptr<StateAssign> _B;
  };

  StateAssignSum::StateAssignSum(std::shared_ptr<StateAssign> A,
      std::shared_ptr<StateAssign> B) : _A(A), _B(B) {
    CHECK_EQ(_A->getLength(), _B->getLength());
    CHECK_EQ(_A->getStateCount(), _B->getStateCount());
  }
}

std::shared_ptr<StateAssign> operator+(std::shared_ptr<StateAssign> A,
                                       std::shared_ptr<StateAssign> B) {
  // Adding an empty pointer is treated as if we added an object with all costs set to 0
  // and amounts to returning the other smart pointer that is possibly non-empty.
  if (!bool(A)) {
    return B;
  }
  if (!bool(B)) {
    return A;
  }

  return std::shared_ptr<StateAssignSum>(new StateAssignSum(A, B));
}


} /* namespace sail */
