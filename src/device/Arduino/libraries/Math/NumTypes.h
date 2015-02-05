/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DEVICE_ARDUINO_LIBRARIES_MATH_NUMTYPES_H_
#define DEVICE_ARDUINO_LIBRARIES_MATH_NUMTYPES_H_

#ifdef ON_SERVER

#endif

/*
 * Common functions for different number types. Should work with fixed point numbers
 * on the microcontroller, but when used on the server, should also work with floating
 * point types and AD types.
 */

// isPositive is necessary when we need to compare
// AD numbers, without having the derivatives ruined.
template <typename T>
bool isPositive(T x) {
  return 0 < x;
}

template <typename T>
template <typename T>
double ToDouble(T x) {return double(x);}

#ifdef ON_SERVER

// Optional support for ADOL-C
#ifdef ADOLC_ADOUBLE_H
template <>
bool isPositive<adouble>(adouble x) {
  return 0 < x.getValue();
}
#endif



#endif


#endif /* DEVICE_ARDUINO_LIBRARIES_MATH_NUMTYPES_H_ */
