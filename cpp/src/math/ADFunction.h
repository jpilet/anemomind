/*
 *  Created on: 2014-01-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ADFUNCTION_H_
#define ADFUNCTION_H_

#include <adolc/adouble.h>
#include "../common/Function.h"

namespace sail
{

// Represents an automatically differentiated function.
class ADFunction : public Function
{
public:
	ADFunction() {}

	// Overrides eval-method of Function
	void eval(double *Xin, double *Fout, double *Jout = nullptr);

	// OVERRIDE THESE METHODS:
	virtual void evalAD(adouble *Xin, adouble *Fout) = 0;
	//	virtual int inDims() = 0;
	//	virtual int outDims() = 0;


	// Can optionally be overridden to provide a custom tape index.
	virtual short int getTapeIndex() {return 0;}



	virtual ~ADFunction() {}
};

} /* namespace sail */

#endif /* ADFUNCTION_H_ */
