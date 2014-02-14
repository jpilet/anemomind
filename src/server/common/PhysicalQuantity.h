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
#define MAKE_PHYSQUANT_TO_UNIT_CONVERTER(name, factor) \
  T name() const {return (factor)*(this->_x);}
#define MAKE_PHYSQUANT_FROM_UNIT_CONVERTER(name, factor) \
  static ThisType name(T x) {return ThisType((factor)*x);}
#define MAKE_PHYSQUANT_UNIT_CONVERTERS(toName, fromName, fromFactor) \
  MAKE_PHYSQUANT_TO_UNIT_CONVERTER(toName, 1.0/(fromFactor)) \
  MAKE_PHYSQUANT_FROM_UNIT_CONVERTER(fromName, (fromFactor))

#define INJECT_COMMON_PHYSQUANT_CODE(ClassName) \
  private: \
    ClassName(T x) : PhysicalQuantity<ClassName<T>, T>(x) {} \
  public: \
    typedef ClassName<T> ThisType; \
    ClassName() : PhysicalQuantity<ClassName<T>, T>() {} \
    static ThisType makeFromX(T x) {return ThisType(x);}

template <typename Quantity, typename Value>
class PhysicalQuantity {
 public:
  typedef Quantity QuantityType;
  typedef Value ValueType;

  Value get() const {return _x;}

  // Additon/subtraction --> Quantity
  Quantity operator+(Quantity other) const {return Quantity::makeFromX(_x + other.get());}
  Quantity operator-(Quantity other) const {return Quantity::makeFromX(_x - other.get());}

  // Scaling by a dimensionless unit --> Quantity
  Quantity operator*(Value x) const {return Quantity::makeFromX(x*_x);}

  // Comparison --> bool
  bool operator < (Quantity other) const {return _x < other._x;}
  bool operator > (Quantity other) const {return _x > other._x;}

  // Multiplication with dimensionless quantity -1 --> Quantity
  Quantity operator - () const {return Quantity::makeFromX(-_x);}

  // Division with a dimensionless quantity --> Quantity
  Quantity operator/ (Value x) const {return Quantity::makeFromX(_x/x);}

  // Division with another quantity --> Value
  Value operator/ (Quantity other) const {return _x/other._x;}
 protected:
  PhysicalQuantity(Value x) : _x(x) {}
  PhysicalQuantity() : _x(Init<Value>::value) {}
  Value _x;
};

// Will the compiler understand this :-D ?
// I constrain the right-hand type to be a PhysicalQuantity
template <typename DimensionlessType, typename Quantity>
Quantity operator*(DimensionlessType s, PhysicalQuantity<Quantity, typename Quantity::ValueType> x) {
  return x*s; // <-- call the operator* method of x.
}

template <typename T>
class Angle : public PhysicalQuantity<Angle<T>, T> {
  INJECT_COMMON_PHYSQUANT_CODE(Angle)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toRadians, radians, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toDegrees, degrees, M_PI/180.0);
};


template <typename T>
class Length : public PhysicalQuantity<Length<T>, T> {
  INJECT_COMMON_PHYSQUANT_CODE(Length)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMeters, meters, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toKilometers, kilometers, 1000.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toNauticalMiles, nauticalMiles, 1852.0);
};

template <typename T>
class Velocity : public PhysicalQuantity<Velocity<T>, T> {
  INJECT_COMMON_PHYSQUANT_CODE(Velocity)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMetersPerSecond, metersPerSecond, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toKnots, knots, 1852/3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toKilometersPerHour, kilometersPerHour, 1.0/3.6);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMilesPerHour, milesPerHour, 0.44704);
};

template <typename T>
class Time : public PhysicalQuantity<Time<T>, T> {
  INJECT_COMMON_PHYSQUANT_CODE(Time)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toSeconds, seconds, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toMinutes, minutes, 60.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toHours, hours, 3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toDays, days, 24*3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toWeeks, weeks, 7*24*3600.0);
};

template <typename T>
class Mass : public PhysicalQuantity<Mass<T>, T> {
  INJECT_COMMON_PHYSQUANT_CODE(Mass)
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toKilograms, kilograms, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toSkeppund, skeppund, 170.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(toLispund, lispund, 170.0/20.0);
};

}

template <typename T>
T cos(sail::Angle<T> x) {return cos(x.get());}

template <typename T>
T sin(sail::Angle<T> x) {return sin(x.get());}




#endif /* DIMENSIONS_H_ */
