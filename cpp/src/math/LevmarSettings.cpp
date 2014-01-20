/*
 * LevmarSettings.cpp
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#include "LevmarSettings.h"

namespace sail
{

LevmarSettings::LevmarSettings()
{
	maxiter = 120;
	e1 = 1.0e-15;
	e2 = 1.0e-15;
	e3 = 1.0e-15;
	tau = 1.0e-3;
	verbosity = 0;
}

void LevmarSettings::vis(Arrayd X)
{
	if (bool(drawf))
	{
		drawf(X.ptr());
	}
}


void LevmarSettings::setDrawf(int inDims, std::function<void(Arrayd)> fun)
{
	drawf = [=] (double *x) {fun(Arrayd(inDims, x));};
}

} /* namespace sail */
