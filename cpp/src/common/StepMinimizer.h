/*
 * StepMinimizer.h
 *
 *  Created on: 21 janv. 2014
 *      Author: jonas
 *
 *
 * Minimizes a function
 * 	f: R -> R
 * by trying discrete steps and gradually reducing the step size.
 */

#ifndef STEPMINIMIZER_H_
#define STEPMINIMIZER_H_

#include <functional>

namespace sail
{

class StepMinimizerState
{
public:
	StepMinimizerState(double x, double step, double value) : _x(x), _step(step), _value(value) {}

	double getX() {return _x;}
	double getStep() {return _step;}
	double getValue() {return _value;}

	void reduceStep() {_step *= 0.5;}
	void increaseStep() {_step *= 2.0;}

	double getLeft() {return _x - _step;}
	double getRight() {return _x + _step;}

	StepMinimizerState make(double x, double value) {return StepMinimizerState(x, _step, value);}
private:
	double _x, _step, _value;
};

class StepMinimizer
{
public:
	StepMinimizer();

	StepMinimizerState takeStep(StepMinimizerState state, std::function<double(double)> fun);
	StepMinimizerState minimize(StepMinimizerState state, std::function<double(double)> fun);
private:
	double _leftLimit, _rightLimit;
	int _maxIter;
};





}

#endif /* STEPMINIMIZER_H_ */
