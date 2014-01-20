/*
 * ADFunction.h
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#ifndef ADFUNCTION_H_
#define ADFUNCTION_H_

#include <adolc/adouble.h>

namespace sail
{

// Represents an automatically differentiated function.
class ADFunction
{
public:
	ADFunction();

	virtual int inDims() = 0;
	virtual int outDims() = 0;
	virtual void eval(adouble *Xin, adouble *Fout) = 0;

	virtual ~ADFunction();
};

} /* namespace sail */

#endif /* ADFUNCTION_H_ */
