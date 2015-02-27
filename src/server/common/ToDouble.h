/*
 *  Created on: 2014-01-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TODOUBLE_H_
#define TODOUBLE_H_

/*
 * I M P O R T A N T
 *
 * Please include this header file AFTER any header file
 * that declares a type for for which ToDouble should be specialized.
 *
 */


namespace {

// Specialize this template for various Auto Diff types
// to avoid non-differentiability.
template <typename T>
double ToDouble(T x) {
  return double(x);
}

// Optional support for ADOL-C
#ifdef ADOLC_ADOUBLE_H
template <>
double ToDouble<adouble>(adouble x) {return x.getValue();}
#endif


// http://ceres-solver.org/nnls_modeling.html#DynamicAutoDiffCostFunction
// Specialization for the default value of the Stride template parameter.
#ifdef CERES_PUBLIC_CERES_H_
template <>
double ToDouble<ceres::Jet<double, 4> >(ceres::Jet<double, 4> x) {
  return x.a;
}
#endif

}
#endif /* TODOUBLE_H_ */
