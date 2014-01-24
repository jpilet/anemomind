/*
 * StateAssign.h
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#ifndef STATEASSIGN_H_
#define STATEASSIGN_H_

#include "Array.h"
#include "MDArray.h"

namespace sail
{

/*
 * This class models a problem of dynamic programming,
 * related to recovering the hidden states of an HMM.
 *
 * The solve() method returns a sequence of indices to these states.
 *
 */
class StateAssign
{
public:
	StateAssign() {}
	virtual ~StateAssign() {}

	// This method returns the cost of being in a particular state 'stateIndex' at a particular time 'timeIndex'.
	virtual double getStateCost(int stateIndex, int timeIndex) = 0;

	// This method returns the cost of going from state 'fromStateIndex' at time 'fromTimeIndex'
	// to state 'toStateIndex' at time 'fromTimeIndex+1'
	virtual double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) = 0;

	// This method returns the number of states that can be assigned.
	// Those states have indices in the range 0..(getStateCount() - 1)
	virtual int getStateCount() = 0;

	// This method returns the length of the sequence for which we should assign states.
	virtual int getLength() = 0;

	// This method returns an array of possible predecessors
	// of a state 'stateIndex' at a time 'timeIndex'.
	virtual Arrayi getPrecedingStates(int stateIndex, int timeIndex) = 0;

	// Computes an optimal state assignment with this
	// for the problem specified by this object.
	Arrayi solve();

	// Lists all state indices, 0..(getStateCount() - 1). This list is suitable to return from
	// the method getPrecedingStates.
	Arrayi listStateInds();
private:
	void accumulateCosts(MDArray2d *costsOut, MDArray2i *ptrsOut);
	double calcBestPred(MDArray2d costs, Arrayi preds, int toState, int fromTime,
			int *bestPredIndexOut);
	Arrayi unwind(MDArray2d costs, MDArray2i ptrs);
};

} /* namespace sail */

#endif /* STATEASSIGN_H_ */
