/*
 * ADFunction.h
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#ifndef ADFUNCTION_H_
#define ADFUNCTION_H_

#include <adolc/adouble.h>
#include "../common/Function.h"

namespace sail
{

// Represents an automatically differentiated function.
// See manual for details about the underlying library used:
//      https://projects.coin-or.org/ADOL-C/browser/stable/2.1/ADOL-C/doc/adolc-manual.pdf
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




	// SEE MANUAL https://projects.coin-or.org/ADOL-C/browser/stable/2.1/ADOL-C/doc/adolc-manual.pdf
	//   on page 5 where it says: "unless several tapes need to be kept, tag = 0 may be used throughout"
	virtual short int getTapeTag() {return 0;}



	virtual ~ADFunction() {}
};

} /* namespace sail */

#endif /* ADFUNCTION_H_ */
