/*
 * StepMinimizer.cpp
 *
 *  Created on: 21 janv. 2014
 *      Author: jonas
 */

#include "StepMinimizer.h"
#include <iostream>

namespace sail
{


namespace
{
	// Attempts a candidate value for X and if it yields an improvement, accepts it as the best state.
	// Returns true being equivalent to an improvement.
	bool attemptStep(double candX,
					double valueAtCandX,

			double &newLimitOutput,
			double &newOppositeLimitOutput,
			const StepMinimizerState &bestStateInput, StepMinimizerState &bestStateOutput)
	{
		if (valueAtCandX < bestStateInput.getValue())
		{
			newOppositeLimitOutput = bestStateInput.getX();
			bestStateOutput = bestStateInput.make(candX, valueAtCandX);
			return true;
		}
		else
		{
			bestStateOutput = bestStateInput;
			newLimitOutput = candX;
			return false;
		}
	}

	#define STEPSTATUS(LABEL, X, LEFT, RIGHT) (std::cout << (LABEL) << ": " << #X << " = " << (X) << "  " << #LEFT << " = " << (LEFT) << "  " << #RIGHT << " = " << (RIGHT) << std::endl)

	bool iterateWithCurrentStepSize(StepMinimizerState &state,
			double &leftLimit, double &rightLimit, std::function<double(double)> fun)
	{
		bool atLeastOneReduction = false;

		// Given the current step size, try to take steps to reduce the function until it is no longer possible
		bool reduced = false;
		do
		{
			reduced = false; // Reset this flag for every iteration.
			double rightX = state.getRight();
			if (rightX < rightLimit)
			{
				reduced |= attemptStep(rightX, fun(rightX),
										rightLimit, leftLimit,
										state, state);
			}

			double leftX = state.getLeft();
			if (leftLimit < leftX)
			{
				reduced |= attemptStep(leftX, fun(leftX),
										leftLimit, rightLimit,
										state, state);
			}
			atLeastOneReduction |= reduced;
		}
		while (reduced); // continue stepping as long as we are able to reduce

		return atLeastOneReduction; // Returns true if there was at lea
	}
}

StepMinimizer::StepMinimizer()
{
	const double maxv = 1.0e30;
	_maxIter = 30;
	_leftLimit = -maxv;
	_rightLimit = maxv;
}


StepMinimizerState StepMinimizer::takeStep(StepMinimizerState state, std::function<double(double)> fun)
{
	double leftLimit = _leftLimit;
	double rightLimit = _rightLimit;

	// Optimistically increase the step size in case 'fun' is somewhat different since the last time we called takeStep.
	// If this is not the case and we simply want to minimize 'fun', use the minimize method instead.
	state.increaseStep();

	for (int i = 0; i < _maxIter; i++)
	{
		// If we are able to reduce the objective function, we say this step is done.
		if (iterateWithCurrentStepSize(state, leftLimit, rightLimit, fun))
		{
			break;
		}

		// otherwise, let's try with a smaller stepsize.
		state.reduceStep();
	}
	return state;
}

StepMinimizerState StepMinimizer::minimize(StepMinimizerState state, std::function<double(double)> fun)
{
	double leftLimit = _leftLimit;
	double rightLimit = _rightLimit;

	for (int i = 0; i < _maxIter; i++)
	{
		iterateWithCurrentStepSize(state, leftLimit, rightLimit, fun);
		state.reduceStep();
	}
	return state;
}



} /* namespace sail */
