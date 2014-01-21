/*
 * ADFunction.cpp
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#include "ADFunction.h"
#include "armaadolc.h"
#include <adolc/taping.h>

namespace sail
{

void ADFunction::eval(double *Xin, double *Fout, double *Jout)
{
	int indims = inDims();
	int outdims = outDims();

	short int tag = getTapeIndex();
	trace_on(tag);
		Arrayad adX(indims);
		adolcInput(indims, adX.getData(), Xin);

		Arrayad adF(outdims);
		evalAD(adX.getData(), adF.getData());

		adolcOutput(outdims, adF.getData(), Fout);
	trace_off();
	if (Jout != nullptr)
	{
		outputJacobianColMajor(tag, Xin, Jout);
	}
}

} /* namespace sail */
