/*
 * LevmarSettings.h
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#ifndef LEVMARSETTINGS_H_
#define LEVMARSETTINGS_H_

#include <functional>
#include "../common/Array.h"
#include <armadillo>

namespace sail
{

typedef std::function<void(double*)> LevmarDrawf;

class LevmarSettings
{
public:
	LevmarSettings();
	int maxiter;
	double e1;
	double e2;
	double e3;
	double tau;
	int verbosity;
	void vis(Arrayd X);
	void setDrawf(int inDims, std::function<void(Arrayd)> fun);
	LevmarDrawf drawf;

	virtual ~LevmarSettings() {}
};

} /* namespace sail */

#endif /* LEVMARSETTINGS_H_ */
