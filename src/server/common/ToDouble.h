/*
 *  Created on: 2014-01-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TODOUBLE_H_
#define TODOUBLE_H_

namespace {

// Specialize this template for various Auto Diff types
// to avoid non-differentiability.
template <typename T>
double ToDouble(T x) {return double(x);}

// Optional support for ADOL-C
#ifdef ADOLC_ADOUBLE_H
template <>
double ToDouble<adouble>(adouble x) {return x.getValue();}
#endif

}

#endif /* TODOUBLE_H_ */
