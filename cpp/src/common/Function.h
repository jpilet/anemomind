/*
 * Function.h
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#ifndef FUNCTION_H_
#define FUNCTION_H_

namespace sail
{

class Function
{
public:
	Function() {}

	// A function
	virtual int inDims() = 0;
	virtual int outDims() = 0;
	virtual void eval(double *Xin, double *Fout, double *Jout = nullptr) = 0;

	void evalJNum(double *Xin, double *JNumOut, double h = 1.0e-6);

	virtual ~Function() {}
};

} /* namespace sail */

#endif /* FUNCTION_H_ */
