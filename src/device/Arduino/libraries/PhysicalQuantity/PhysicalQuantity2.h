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

  typedef PhysicalQuantity<T, TimeDim, LengthDim, AngleDim, MassDim> ThisType;

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
  MAKE_PHYSQUANT_VELOCITY_CONVERTERS(milesPerHour, 1609.0/3600.0);

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

  static PhysicalQuantity<T, TimeDim, LengthDim, AngleDim, MassDim> wrap(T x) {
    return PhysicalQuantity<T, TimeDim, LengthDim, AngleDim, MassDim>(x);
  }



  template <typename T, int t, int l, int a, int m>
  PhysicalQuantity<T, TimeDim + t, LengthDim + l, AngleDim + a, MassDim + m> operator*(
      const PhysicalQuantity<T, t, l, a, m> &other) {
    PhysicalQuantity<T, TimeDim + t, LengthDim + l, AngleDim + a, MassDim + m>(_x*other._x);
  }

  ThisType operator*(T s) const {
    return ThisType(s*_x);
  }

  ThisType operator+(const ThisType &other) const {
    return ThisType(_x + other._x);
  }
private:
  PhysicalQuantity(T x) : _x(x) {}
  T _x;
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

template <typename T, int t, int l, int a, int m>
PhysicalQuantity<T, t, l, a, m> operator*(T s, const PhysicalQuantity<T, t, l, a, m> &x) {
  return x*s;
}

}



#endif /* DEVICE_ARDUINO_LIBRARIES_PHYSICALQUANTITY_PHYSICALQUANTITY2_H_ */
