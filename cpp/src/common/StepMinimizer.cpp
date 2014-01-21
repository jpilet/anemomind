/*
 * StepMinimizer.cpp
 *
 *  Created on: 21 janv. 2014
 *      Author: jonas
 */

#include "StepMinimizer.h"

namespace sail
{


namespace
{
	void attemptStep(double candX, double &limit, StepMinimizerState &bestState, double value, bool &reduced,
		std::function<bool(double, double)> acceptor)
	{
		if (value < bestState.getValue() && (bool(acceptor)? acceptor(candX, value) : true))
		{
			bestState = bestState.make(candX, value);
			reduced = true;
		}
		else
		{
			limit = candX; // don't go beyond this point anymore
		}
	}



	bool iterateWithCurrentStepSize(StepMinimizerState &state,
			double &leftLimit, double &rightLimit, std::function<double(double)> fun,
			std::function<bool(double, double)> acceptor)
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
				attemptStep(rightX, rightLimit, state, fun(rightX), reduced, acceptor);
			}

			double leftX = state.getLeft();
			if (leftLimit < leftX)
			{
				attemptStep(leftX, leftLimit, state, fun(leftX), reduced, acceptor);
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
		if (iterateWithCurrentStepSize(state, leftLimit, rightLimit, fun, _acceptor))
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
		iterateWithCurrentStepSize(state, leftLimit, rightLimit, fun, _acceptor);
		state.reduceStep();
	}
	return state;
}

void StepMinimizer::setAcceptor(std::function<bool(double, double)> acceptor)
{
	_acceptor = acceptor;
}



} /* namespace sail */
