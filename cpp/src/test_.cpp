#include "gtest/gtest.h"
#include "common/ContinuousRange.h"
#include <cmath>
#include "common/StepMinimizer.h"
#include "common/common.h"
#include "math/ADFunction.h"
#include "math/LevMar.h"
#include "math/LevmarSettings.h"
#include "common/math.h"

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


class LMTestFun : public ADFunction
{
public:
	int inDims() {return 2;}
	int outDims() {return 2;}
	void evalAD(adouble *Xin, adouble *Fout);
private:
};

void LMTestFun::evalAD(adouble *Xin, adouble *Fout)
{
	Fout[0] = sqrt(sqr(Xin[0]) + sqr(Xin[1])) - 1.0;
	Fout[1] = sqrt(sqr(Xin[0] - 1.0) + sqr(Xin[1] - 1.0)) - 1.0;
}

// Compute one of the two intersections between two circles with radi 1.0,
// located at (0, 0) and (1, 1)
TEST(OptimizationTest, CircleFit)
{
	arma::mat Xinit(2, 1);
	Xinit(0, 0) = 3.0;
	Xinit(1, 0) = 1.0;

	LMTestFun objf;
	LevmarState state(Xinit);
	LevmarSettings settings;
	state.minimize(settings, objf);

	double tol = 1.0e-6;
	double *x = state.getX().memptr();
	EXPECT_TRUE((std::abs(x[0]) < tol && std::abs(x[1] - 1.0) < tol) ||
				(std::abs(x[1]) < tol && std::abs(x[0] - 1.0) < tol));
}

TEST(OptimizationTest, NumericJacobian)
{
	arma::mat Xinit(2, 1);
	Xinit(0, 0) = 3.0;
	Xinit(1, 0) = 1.0;

	LMTestFun objf;
	arma::mat F(objf.outDims(), 1);
	arma::mat J(objf.outDims(), objf.inDims());
	arma::mat Jnum(objf.outDims(), objf.inDims());

	objf.evalJNum(Xinit.memptr(), Jnum.memptr());
	objf.eval(Xinit.memptr(), F.memptr(), J.memptr());

	arma::mat dif = Jnum - J;
	for (int i = 0; i < objf.outDims(); i++)
	{
		for (int j = 0; j < objf.inDims(); j++)
		{
			EXPECT_LE(std::abs(dif(i, j)), 1.0e-3);
		}
	}
}
