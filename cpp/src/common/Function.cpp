/*
 * Function.cpp
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#include "Function.h"
#include  "MDArray.h"

namespace sail
{

void Function::evalJNum(double *Xin, double *JNumOut, double h)
{
	int M = inDims();
	int N = outDims();
	Arrayd Fplus(N), Fminus(N);
	MDArray2d Jnum(M, N, JNumOut);

	double oneOver2h = 1.0/(2.0*h);
	for (int j = 0; j < M; j++)
	{
		double bak = Xin[j];
		Xin[j] = bak + h;
		eval(Xin, Fplus.getData());
		Xin[j] = bak - h;
		eval(Xin, Fminus.getData());
		Xin[j] = bak;
	}
}

} /* namespace sail */
