/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  HOW TO USE THESE CLASSES
 *  ========================
 *  These classes are useful in public APIs for
 *    * Parameter passing and return values of public functions
 *    * Instance variables of objects
 *
 *  In the hidden or low-level parts of the implementation,
 *  it may be more convenient to use raw floating point values.
 */


#ifndef DIMENSIONS_H_
#define PHYSICALQUANTITY_H_

#include <cmath>

namespace sail {

template <typename T>
class Init {
 public:
  static const T value = NAN;
};

/*
 * These macros may not be in conformity with the
 * coding style guide but they reduce code duplication effort
 * and make the code much more readable than it otherwise would be.
 */
#define MAKE_PHYSQUANT_TO_UNIT_CONVERTER(name, factor) T name() const {return (factor)*_x;}
#define MAKE_PHYSQUANT_FROM_UNIT_CONVERTER(name, factor) static ThisType name(T x) {return ThisType((factor)*x);}
#define MAKE_PHYSQUANT_UNIT_CONVERTERS(toName, fromName, fromFactor) MAKE_PHYSQUANT_TO_UNIT_CONVERTER(toName, 1.0/(fromFactor)) MAKE_PHYSQUANT_FROM_UNIT_CONVERTER(fromName, (fromFactor))
#define INJECT_COMMON_PHYSQUANT_CODE(ClassName) private: T _x; ClassName(T x) : _x(x) {} public: T &raw() {return _x;} typedef ClassName<T> ThisType; ClassName() : _x(Init<T>::value) {}


template <typename T>
class Angle {
  INJECT_COMMON_PHYSQUANT_CODE(Angle)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toRadians, fromRadians, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toDegrees, fromDegrees, M_PI/180.0);
};


template <typename T>
class Length {
  INJECT_COMMON_PHYSQUANT_CODE(Length)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMetres, fromMetres, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toKilometres, fromKilometres, 1000.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toNauticalMiles, fromNauticalMiles, 1852.0);
};

template <typename T>
class Velocity {
  INJECT_COMMON_PHYSQUANT_CODE(Velocity)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMetresPerSecond, fromMetresPerSecond, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toKnots, fromKnots, 1852/3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toKilometresPerHour, fromKilometresPerHour, 1.0/3.6);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMilesPerHour, fromMilesPerHour, 0.44704);
};

template <typename T>
class Time {
  INJECT_COMMON_PHYSQUANT_CODE(Time)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toSeconds, fromSeconds, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMinutes, fromMinutes, 60.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toHours, fromHours, 3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toDays, fromDays, 24*3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toWeeks, fromWeeks, 7*24*3600.0);
};
}

template <typename T>
T cos(sail::Angle<T> x) {return cos(x.raw());}

template <typename T>
T sin(sail::Angle<T> x) {return sin(x.raw());}




#endif /* DIMENSIONS_H_ */
