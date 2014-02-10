/*
 * LevmarTest.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */
#include "LevMar.h"
#include "LevmarSettings.h"
#include <gtest/gtest.h>
#include <server/common/math.h>
#include <server/math/ADFunction.h>
#include <server/math/nonlinear/StepMinimizer.h>

using namespace sail;

namespace
{
	class LMTestFun : public AutoDiffFunction
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
}

// Compute one of the two intersections between two circles with radi 1.0,
// located at (0, 0) and (1, 1)
TEST(OptimizationTest, CircleFit)
{
	const int startCount = 5;

	double I[startCount] = {3.0, 4.0, 17.0, 1.5, -10};
	double J[startCount] = {1.0, 2.0, 0.5, 4.9, -12};

	for (int i = 0; i < startCount; i++)
	{
		for (int j = 0; j < startCount; j++)
		{
			arma::mat Xinit(2, 1);
			Xinit(0, 0) = I[i];
			Xinit(1, 0) = J[j];

			LMTestFun objf;
			LevmarState state(Xinit);
			LevmarSettings settings;
			state.minimize(settings, objf);

			double tol = 1.0e-6;
			double *x = state.getX().memptr();

			double xmin = std::min(x[0], x[1]);
			double xmax = std::max(x[0], x[1]);
			EXPECT_NEAR(0, xmin, tol);
			EXPECT_NEAR(1, xmax, tol);
		}
	}
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

	objf.evalNumericJacobian(Xinit.memptr(), Jnum.memptr());
	objf.eval(Xinit.memptr(), F.memptr(), J.memptr());

	for (int i = 0; i < objf.outDims(); i++)
	{
		for (int j = 0; j < objf.inDims(); j++)
		{
			EXPECT_NEAR(Jnum(i, j), J(i, j), 1.0e-5);
		}
	}
}

namespace
{
	class Function3x2 : public AutoDiffFunction
	{
	public:
		void evalAD(adouble *Xin, adouble *Fout);
		int inDims() {return 2;}
		int outDims() {return 3;}
	};

	void Function3x2::evalAD(adouble *Xin, adouble *Fout)
	{
		Fout[0] = 2*Xin[0] + 3*exp(Xin[1]);
		Fout[1] = sqrt(Xin[0]) + 4.0*sin(Xin[1]);
		Fout[2] = Xin[0]*Xin[1] + Xin[1]*Xin[1];
	}
}


TEST(OptimizationTest, NumericJacobian3x2)
{
	arma::mat Xinit(2, 1);
	Xinit(0, 0) = 3.0;
	Xinit(1, 0) = 1.0;

	Function3x2 objf;
	arma::mat F(objf.outDims(), 1);
	arma::mat J(objf.outDims(), objf.inDims());
	arma::mat Jnum(objf.outDims(), objf.inDims());

	objf.evalNumericJacobian(Xinit.memptr(), Jnum.memptr());
	objf.eval(Xinit.memptr(), F.memptr(), J.memptr());

	for (int i = 0; i < objf.outDims(); i++)
	{
		for (int j = 0; j < objf.inDims(); j++)
		{
			EXPECT_NEAR(Jnum(i, j), J(i, j), 1.0e-5);
		}
	}
}
