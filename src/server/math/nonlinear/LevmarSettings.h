/*
 * LevmarSettings.h
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#ifndef LEVMARSETTINGS_H_
#define LEVMARSETTINGS_H_

#include <functional>
#include <server/common/Array.h>
#include <armadillo>

namespace sail
{


class LevmarSettings
{
public:
	LevmarSettings();
	int maxiter;

	// Please see
	// A Brief Description of the Levenberg-Marquardt Algorithm
	//   by Manolis I. A. Lourakis
	// http://users.ics.forth.gr/~lourakis/levmar/levmar.pdf
	// 'e1', 'e2' and 'e3' stand for epsilon
	double e1;
	double e2;
	double e3;
	double tau;

	int verbosity;
	void draw(Arrayd X);

	void setDrawf(int inDims, std::function<void(Arrayd)> fun);

	// Optional function to visualize the result.
	std::function<void(double*)> drawf;

	// Function used to perform an additional test whether to accept a step,
	// in addition to the requirement that 'X' has to yield a 'value' less than
	// previous value. By default, this function is not set and ignored.
	// The acceptor function is useful if we have additional criteria on the solution,
	//   for instance that it should be pareto efficient.
	std::function<bool(double *X, double value)> acceptor;
};

} /* namespace sail */

#endif /* LEVMARSETTINGS_H_ */
