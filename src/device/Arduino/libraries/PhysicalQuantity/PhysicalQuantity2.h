/*
 * PhysicalQuantity2.h
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#ifndef DEVICE_ARDUINO_LIBRARIES_PHYSICALQUANTITY_PHYSICALQUANTITY2_H_
#define DEVICE_ARDUINO_LIBRARIES_PHYSICALQUANTITY_PHYSICALQUANTITY2_H_

#include <math.h>

namespace sail {

#define MAKE_PHYSQUANT_UNIT_CONVERTERS(t, l, a, m, name, fromFactor) \
    static PhysicalQuantity<T, t, l, a, m> name(T x) { \
      return PhysicalQuantity<T, t, l, a, m>(T(fromFactor)*x); \
    } \
  T name() const { \
    static_assert(TimeDim == t, "Not applicable on this type"); \
    static_assert(LengthDim == l, "Not applicable on this type"); \
    static_assert(AngleDim == a, "Not applicable on this type"); \
    static_assert(MassDim == m, "Not applicable on this type"); \
    return (T(1.0) / T(fromFactor))*(this->_x); \
  }


#define MAKE_PHYSQUANT_TIME_CONVERTERS(name, fromFactor) \
    MAKE_PHYSQUANT_UNIT_CONVERTERS(1, 0, 0, 0, name, fromFactor)

#define MAKE_PHYSQUANT_LENGTH_CONVERTERS(name, fromFactor) \
    MAKE_PHYSQUANT_UNIT_CONVERTERS(0, 1, 0, 0, name, fromFactor)

#define MAKE_PHYSQUANT_VELOCITY_CONVERTERS(name, fromFactor) \
    MAKE_PHYSQUANT_UNIT_CONVERTERS(0, 1, 0, 0, name, fromFactor)

#define MAKE_PHYSQUANT_ANGLE_CONVERTERS(name, fromFactor) \
    MAKE_PHYSQUANT_UNIT_CONVERTERS(0, 0, 1, 0, name, fromFactor)

#define MAKE_PHYSQUANT_MASS_CONVERTERS(name, fromFactor) \
    MAKE_PHYSQUANT_UNIT_CONVERTERS(0, 0, 0, 1, name, fromFactor)

template <typename T, int TimeDim/*t*/, int LengthDim/*l*/, int AngleDim/*a*/, int MassDim/*m*/>
class PhysicalQuantity {
public:
  PhysicalQuantity() : _x(T(NAN)) {}

  static PhysicalQuantity<T, 0, 0, 0, 0> scalar(T x) {return PhysicalQuantity<T, 0, 0, 0>(x);}

  MAKE_PHYSQUANT_TIME_CONVERTERS(milliseconds, 0.001)
  MAKE_PHYSQUANT_TIME_CONVERTERS(seconds, 1.0);
  MAKE_PHYSQUANT_TIME_CONVERTERS(minutes, 60.0);
  MAKE_PHYSQUANT_TIME_CONVERTERS(hours, 3600.0);
  MAKE_PHYSQUANT_TIME_CONVERTERS(days, 24*3600.0);
  MAKE_PHYSQUANT_TIME_CONVERTERS(weeks, 7*24*3600.0);

  MAKE_PHYSQUANT_VELOCITY_CONVERTERS(knots, 1852.0/3600.0)
  MAKE_PHYSQUANT_VELOCITY_CONVERTERS(metersPerSecond, 1.0);
  MAKE_PHYSQUANT_VELOCITY_CONVERTERS(kilometersPerHour, 1000.0/3600.0);

  MAKE_PHYSQUANT_MASS_CONVERTERS(kilograms, 1.0);
  MAKE_PHYSQUANT_MASS_CONVERTERS(skeppund, 170.0);
  MAKE_PHYSQUANT_MASS_CONVERTERS(lispund, 170.0/20.0);

  MAKE_PHYSQUANT_LENGTH_CONVERTERS(meters, 1.0)
  MAKE_PHYSQUANT_LENGTH_CONVERTERS(kilometers, 1000.0);
  MAKE_PHYSQUANT_LENGTH_CONVERTERS(nauticalMiles, 1852.0);

  operator T () const {
    static_assert(TimeDim == 0, "Only dimensionless units");
    static_assert(LengthDim == 0, "Only dimensionless units");
    static_assert(AngleDim == 0, "Only dimensionless units");
    static_assert(MassDim == 0, "Only dimensionless units");
    return _x;
  }
private:
  T _x;
  PhysicalQuantity(T x) : _x(x) {}
};


template <typename T>
using Duration = PhysicalQuantity<T, 1, 0, 0, 0>;

template <typename T>
using Length = PhysicalQuantity<T, 0, 1, 0, 0>;

template <typename T>
using Velocity = PhysicalQuantity<T, -1, 1, 0, 0>;

template <typename T>
using Angle = PhysicalQuantity<T, 0, 0, 1, 0>;

template <typename T>
using Mass = PhysicalQuantity<T, 0, 0, 0, 1>;

}



#endif /* DEVICE_ARDUINO_LIBRARIES_PHYSICALQUANTITY_PHYSICALQUANTITY2_H_ */
