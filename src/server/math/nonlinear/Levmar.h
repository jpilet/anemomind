/*
 * LevMar.h
 *
 *  Created on: 21 janv. 2014
 *      Author: jonas
 */

#ifndef LEVMAR_H_
#define LEVMAR_H_

#include <server/common/Function.h>
#include <armadillo>

namespace sail
{

class LevmarSettings;
class LevmarState
{
public:
	LevmarState(arma::mat X);

	// Take a single step
	void step(const LevmarSettings &settings, Function &fun);

	void minimize(const LevmarSettings &settings, Function &fun);

	arma::mat &getX() {return _X;}
private:
	arma::mat _X;
	double _v, _mu;
	arma::mat JtJ, JtF;
	bool _stop;

	arma::mat _Fscratch, _Jscratch;
};

} /* namespace sail */

#endif /* LEVMAR_H_ */
