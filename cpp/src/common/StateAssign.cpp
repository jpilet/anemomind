/*
 * StateAssign.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "StateAssign.h"
#include "common.h"
#include "ArrayIO.h"

namespace sail
{

Arrayi StateAssign::solve()
{
	MDArray2d costs;
	MDArray2i ptrs;
	accumulateCosts(&costs, &ptrs);
	return unwind(costs, ptrs);
}

Arrayi StateAssign::listStateInds()
{
	return makeRange(getStateCount());
}

void StateAssign::accumulateCosts(MDArray2d *costsOut, MDArray2i *ptrsOut)
{
	int length = getLength();
	int stateCount = getStateCount();
	MDArray2d costs(stateCount, length);
	MDArray2i ptrs(stateCount, length);
	costs.setAll(0.0);
	ptrs.setAll(-1);
	for (int i = 0; i < stateCount; i++)
	{
		costs(i, 0) = getStateCost(i, 0);
	}

	Arrayi temp(stateCount);
	for (int j = 1; j < length; j++) // For every time index >= 1
	{
		for (int i = 0; i < stateCount; i++) // For every state at that time index
		{
			int bestPredIndex = -1;
			Arrayi preds = getPrecedingStates(i, j);
			costs(i, j) = getStateCost(i, j) + calcBestPred(costs.sliceCol(j-1), preds, i, j-1, &bestPredIndex);
			assert(bestPredIndex != -1);
			ptrs(i, j) = bestPredIndex;
		}
	}

	// Output the results
	*costsOut = costs;
	*ptrsOut = ptrs;
}

double StateAssign::calcBestPred(MDArray2d costs, Arrayi preds, int toState, int fromTime,
		int *bestPredIndexOut)
{
	assert(costs.cols() == 1); // Because it should be a slice of the preceding costs.
	if (preds.empty())
	{
		*bestPredIndexOut = -1;
		return 1.0e9;
	}
	else
	{
		int bestIndex = preds[0];
		double bestCost = costs(bestIndex, 0);
		int count = preds.size();
		for (int i = 1; i < count; i++)
		{
			int index = preds[i];
			double cost = costs(index, 0) + getTransitionCost(index, toState, fromTime);
			if (cost < bestCost)
			{
				bestCost = cost;
				bestIndex = index;
			}
		}
		*bestPredIndexOut = bestIndex;
		return bestCost;
	}
}

namespace
{
	int getLastBestState(MDArray2d costs)
	{
		int last = costs.cols()-1;
		int count = costs.rows();
		int bestIndex = 0;
		double bestCost = costs(0, last);
		for (int i = 1; i < count; i++)
		{
			double cost = costs(i, last);
			if (cost < bestCost)
			{
				bestIndex = i;
			}
		}
		return bestIndex;
	}

}

Arrayi StateAssign::unwind(MDArray2d costs, MDArray2i ptrs)
{
	int stateCount = getStateCount();
	int length = getLength();
	assert(costs.isMatrix(stateCount, length));
	assert(ptrs.isMatrix(stateCount, length));

	Arrayi states(length);
	states.setTo(-1);
	int last = length - 1;
	states[last] = getLastBestState(costs);
	for (int i = last-1; i >= 0; i--)
	{
		int next = i + 1;
		int nextState = states[next];
		int index = ptrs(nextState, next);
		assert(index != -1);
		states[i] = index;
	}
	return states;
}

} /* namespace sail */
