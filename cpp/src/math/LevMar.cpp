/*
 * LevMar.cpp
 *
 *  Created on: 21 janv. 2014
 *      Author: jonas
 */

#include "LevMar.h"
#include "mathutils.h"
#include "LevmarSettings.h"
#include "../common/math.h"

namespace sail
{

LevmarState::LevmarState(arma::mat X)
{
	_X = X;
	_v = 2.0;
	_mu = -1; // Set to negative number to indicate it is not initialized.
	_stop = false;
}

namespace
{
	double maxDiagElement(const arma::mat M)
	{
		int n = std::min(M.n_rows, M.n_cols);
		double maxv = 0.0;
		for (int i = 0; i < n; i++)
		{
			double x = M(i, i);
			if (x > maxv)
			{
				maxv = x;
			}
		}
		return maxv;
	}

	double maxAbsElement(const arma::mat X)
	{
		assert(X.n_cols == 1);
		double maxv = 0.0;
		for (int i = 0; i < X.n_rows; i++)
		{
			double x = std::abs(X(i, 0));
			if (x > maxv)
			{
				maxv = x;
			}
		}
		return maxv;
	}
}

void LevmarState::step(const LevmarSettings &settings, Function &fun)
{
	arma::createMat(_Jscratch, fun.outDims(), fun.inDims());
	arma::createMat(_Fscratch, fun.outDims(), 1);
	fun.eval(_X.memptr(), _Fscratch.memptr(), _Jscratch.memptr());
	_JtJ = _Jscratch.t()*_Jscratch;
	_JtF = _Jscratch.t()*_Fscratch;

	// Initialize _mu, if not already done
	if (_mu < 0)
	{
		_mu = settings.tau*maxDiagElement(_JtJ);
	}

	if (maxAbsElement(_JtF) < settings.e1 || norm2(_Fscratch.n_elem, _Fscratch.memptr()) <= settings.e3)
	{
		LMWRITE(1, "  Stop because optimum reached.");
		_stop = true;
	}
	else
	{
		assert(_mu > 0);
		assert(!std::isnan(_mu));
		assert(!std::isnan(_v));

		arma::mat Xnew;
		Arrayd Ftemp;
		double normX = norm(_X.n_elem, _X.memptr());
		double rho = -1;

		double norm2F = norm2(_Fscratch.n_elem, _Fscratch.memptr());

		while (!(_stop || rho > 0))
		{
			assert(!std::isnan(_mu));
			arma::mat dX = -arma::solve(_JtJ + _mu*arma::eye(_JtJ.n_rows, _JtJ.n_cols), _JtF);
			if (arma::norm(dX, 2) < settings.e2*normX)
			{
				LMWRITE(1, "  Stop because step size is minimum.");
				_stop = true;
			}
			else
			{
				Xnew = _X + dX;
				double norm2Fnew = fun.calcSquaredNorm(Xnew.memptr(), _Fscratch.memptr());


				arma::mat denom = (dX.t()*(_mu*dX - _JtF));
				rho = (norm2F - norm2Fnew)/denom(0, 0);
				if (rho > 0 && // Improvement over previous estimate
						(bool(settings.acceptor)? settings.acceptor(Xnew.memptr(), norm2Fnew) : true))
				{
					LMWRITE(2, "  Improvement with this step size.");
					_X = Xnew;

					_mu = _mu*std::max(1.0/3, 1 - std::pow(2*rho - 1, 3));
					assert(!std::isnan(_mu));
					_v = 2;
				}
				else // Increase damping
				{
					LMWRITE(2, "  Increase damping.");
					_mu = _mu*_v;
					assert(!std::isnan(_mu));
					_v = 2*_v;
					assert(!std::isinf(_v));
				}
			}
		}
	}
}

void LevmarState::minimize(const LevmarSettings &settings, Function &fun)
{
	for (int i = 0; i < settings.maxiter; i++)
	{
		step(settings, fun);
		if (_stop)
		{
			break;
		}
	}
}


} /* namespace sail */
