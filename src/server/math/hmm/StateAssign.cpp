/*
 * StateAssign.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "StateAssign.h"
#include <server/common/ArrayIO.h>
#include <server/common/ArrayBuilder.h>
#include <limits>

namespace sail {

Arrayi StateAssign::solve() {
  MDArray2d costs;
  MDArray2i ptrs;
  accumulateCosts(&costs, &ptrs);
  return unwind(costs, ptrs);
}

MDArray2d StateAssign::makeCostMatrix() {
  MDArray2d costs;
  MDArray2i ptrs;
  accumulateCosts(&costs, &ptrs);
  return costs;
}

MDArray2i StateAssign::makeRefMatrix() {
  MDArray2d costs;
  MDArray2i ptrs;
  accumulateCosts(&costs, &ptrs);
  return ptrs;
}


Arrayi StateAssign::listStateInds() {
  return makeRange(getStateCount());
}

void StateAssign::accumulateCosts(MDArray2d *costsOut, MDArray2i *ptrsOut) {
  int length = getLength();
  int stateCount = getStateCount();
  MDArray2d costs(stateCount, length);
  MDArray2i ptrs(stateCount, length);
  costs.setAll(0.0);
  ptrs.setAll(-1);
  for (int state = 0; state < stateCount; state++) {
    costs(state, 0) = getStateCost(state, 0);
  }

  for (int time = 1; time < length; time++) { // For every time index >= 1
    for (int state = 0; state < stateCount; state++) { // For every state at that time index
      int bestPredIndex = -1;
      Arrayi preds = getPrecedingStates(state, time);
      double stepcost = calcBestPred(costs, preds, state, time-1, &bestPredIndex);
      costs(state, time) = getStateCost(state, time) + stepcost;
      assert(bestPredIndex != -1);
      ptrs(state, time) = bestPredIndex;
    }
  }

  // Output the results
  *costsOut = costs;
  *ptrsOut = ptrs;
}

double StateAssign::calcBestPred(MDArray2d costs, Arrayi preds, int toState, int fromTime,
                                 int *bestPredIndexOut) {
  if (preds.empty()) {
    *bestPredIndexOut = -1;
    return 1.0e9;
  } else {
    int bestIndex = preds[0];
    double bestCost = std::numeric_limits<double>::infinity();
    int count = preds.size();
    for (int state = 0; state < count; state++) {
      int stateIndex = preds[state];
      double cost = costs(stateIndex, fromTime) + getTransitionCost(stateIndex, toState, fromTime);
      if (cost < bestCost) {
        bestCost = cost;
        bestIndex = stateIndex;
      }
    }
    *bestPredIndexOut = bestIndex;
    return bestCost;
  }
}

namespace {
int getLastBestState(MDArray2d costs) {
  int last = costs.cols()-1;
  int count = costs.rows();
  int bestIndex = 0;
  double bestCost = costs(0, last);
  for (int state = 1; state < count; state++) {
    double cost = costs(state, last);
    if (cost < bestCost) {
      bestIndex = state;
    }
  }
  return bestIndex;
}

}

Arrayi StateAssign::unwind(MDArray2d costs, MDArray2i ptrs) {
  int stateCount = getStateCount();
  int length = getLength();
  assert(costs.isMatrix(stateCount, length));
  assert(ptrs.isMatrix(stateCount, length));

  Arrayi states(length);
  states.setTo(-1);
  int last = length - 1;
  states[last] = getLastBestState(costs);
  for (int time = last-1; time >= 0; time--) {
    int next = time + 1;
    int nextState = states[next];
    int index = ptrs(nextState, next);
    assert(index != -1);
    states[time] = index;
  }
  return states;
}

Array<Arrayi> StateAssign::makePredecessorsPerState(MDArray2b con) {
  int n = con.rows();
  assert(con.isSquare());
  Array<Arrayi> preds(n);
  for (int j = 0; j < n; j++) {
    ArrayBuilder<int> builder(n);
    for (int i = 0; i < n; i++) {
      if (con(i, j)) {
        builder.add(i);
      }
    }
    preds[j] = builder.get();
  }
  return preds;
}

} /* namespace sail */
