/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef RUNGEKUTTA_H_
#define RUNGEKUTTA_H_

#include <server/common/Array.h>
#include <server/common/Function.h>
#include <memory>

namespace sail {

/*
 * Basic implementation of Runge-Kutta method,
 *
 * http://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods#The_Runge.E2.80.93Kutta_method
 *
 * assuming no dependence on a time variable.
 */
void takeRungeKuttaStep(std::shared_ptr<Function> fun, Arrayd *stateVector, double stepSize);

}

#endif /* RUNGEKUTTA_H_ */
