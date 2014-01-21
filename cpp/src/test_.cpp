#include "gtest/gtest.h"
#include "common/ContinuousRange.h"
#include <cmath>
#include "common/StepMinimizer.h"
#include "common/common.h"

using namespace sail;

TEST(ContinuousRangeTest, TestIntersects)
{
	EXPECT_TRUE(ContinuousRange(Arrayd::args(1, 2), false).intersects(ContinuousRange(Arrayd::args(1.5, 3), false)));
	EXPECT_FALSE(ContinuousRange(Arrayd::args(1, 2), false).intersects(ContinuousRange(Arrayd::args(3, 4), false)));
	EXPECT_TRUE(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(ContinuousRange(Arrayd::args(0.1 + 4.0*M_PI, 0.3), true)));
	EXPECT_FALSE(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(ContinuousRange(Arrayd::args(0.4 + 4.0*M_PI, 0.31), true)));
	EXPECT_TRUE(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(ContinuousRange(Arrayd::args(0.0, 0.3), true)));
}

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
			double err = std::abs(finalState.getX() - Xopt);
			EXPECT_LE(err, 1.0e-6);
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
			double err = std::abs(state.getX() - Xopt);
			EXPECT_LE(err, 1.0e-6);
		}
	}
}
