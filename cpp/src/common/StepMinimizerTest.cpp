/*
 * StepMinimizerTest.cpp
<<<<<<< HEAD
=======

>>>>>>> master
 *
 *  Created on: 22 janv. 2014
 *      Author: jonas
 */
#include "gtest/gtest.h"
#include "StepMinimizer.h"
#include <cmath>
using namespace sail;

TEST(StepMinimizerTest, MinimizeConvex)
{
	const int coefCount = 3;
	double coefs[coefCount] = {2.0, 3.0, 4.0};

	double initX = 300;
	double initStep = 100;
	for (int i = 0; i < coefCount; i++)
	{
		double a = coefs[i];
		for (int j = 0; j < coefCount; j++)
		{
			double b = coefs[j];
			double c = 1.0;

			auto fun = [&] (double x) {return a*x*x + b*x + c;};

			// 2*a*x + b = 0 <=> 2*a*x = -b <=> x = -b/(2*a)
			double Xopt = -b/(2*a);

			StepMinimizer minimizer;
			StepMinimizerState initialState(initX, initStep, fun(initX));

			StepMinimizerState finalState = minimizer.minimize(initialState, fun);
			EXPECT_NEAR(Xopt, finalState.getX(), 1.0e-6);
		}
	}
}

TEST(StepMinimizerTest, MinimizeConvexStepByStep)
{
	const int coefCount = 3;
	double coefs[coefCount] = {2.0, 3.0, 4.0};

	double initX = 300;
	double initStep = 100;
	for (int i = 0; i < coefCount; i++)
	{
		double a = coefs[i];
		for (int j = 0; j < coefCount; j++)
		{
			double b = coefs[j];
			double c = 1.0;

			auto fun = [&] (double x) {return a*x*x + b*x + c;};

			// 2*a*x + b = 0 <=> 2*a*x = -b <=> x = -b/(2*a)
			double Xopt = -b/(2*a);

			StepMinimizer minimizer;
			StepMinimizerState state(initX, initStep, fun(initX));

			for (int k = 0; k < 30; k++)
			{
				state = minimizer.takeStep(state, fun);
			}
			EXPECT_NEAR(state.getX(), Xopt, 1.0e-6);
		}
	}
}
