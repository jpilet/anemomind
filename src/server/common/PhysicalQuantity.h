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


#ifndef PHYSICALQUANTITY_H_
#define PHYSICALQUANTITY_H_

#include <cmath>
#include <string>
#include <sstream>
#include <limits>

namespace sail {

/*
 * These macros may not be in conformity with the
 * coding style guide but they reduce code duplication effort
 * and make the code much more readable than it otherwise would be.
 */
#define MAKE_PHYSQUANT_UNIT_CONVERTERS(name, fromFactor) \
  T name() const {return (1.0 / (fromFactor))*(this->_x);} \
  static ThisType name(T x) {return ThisType((fromFactor)*x);}

#define DECLARE_PHYSQUANT_CONSTRUCTORS(ClassName) \
  private: \
    ClassName(T x) : PhysicalQuantity<ClassName<T>, T>(x) {} \
  public: \
    typedef ClassName<T> ThisType; \
    ClassName() : PhysicalQuantity<ClassName<T>, T>() { } \
    ClassName(const PhysicalQuantity<ClassName<T>, T> &x) : PhysicalQuantity<ClassName<T>, T>(x) {}

template <typename Quantity, typename Value>
class PhysicalQuantity {
 public:
  const static Value defaultValue;
  typedef PhysicalQuantity<Quantity, Value> ThisQuantity;
  typedef Quantity QuantityType;
  typedef Value ValueType;


  // Additon/subtraction --> Quantity
  Quantity operator+(ThisQuantity other) const {
    return Quantity::makeFromX(_x + other.get());
  }

  ThisQuantity &operator += (ThisQuantity other) {
    _x += other.get();
    return *this;
  }

  Quantity operator-(ThisQuantity other) const {
    return Quantity::makeFromX(_x - other.get());
  }

  ThisQuantity &operator -= (ThisQuantity other) {
    _x -= other.get();
    return *this;
  }


  // Comparison --> bool
  bool operator < (ThisQuantity other) const {return _x < other.get();}
  bool operator <= (ThisQuantity other) const {return _x <= other.get();}
  bool operator > (ThisQuantity other) const {return _x > other.get();}
  bool operator >= (ThisQuantity other) const {return _x >= other.get();}

 protected:
  Value get() const {return _x;}
  PhysicalQuantity(Value x) : _x(x) {}
  PhysicalQuantity() : _x(Quantity::defaultValue) {}
  Value _x;
 private:
  static Quantity makeFromX(Value X) { return Quantity(PhysicalQuantity<Quantity, Value>(X)); }
};

template <typename Quantity, typename Value>
const Value PhysicalQuantity<Quantity, Value>::defaultValue =
    Value(std::numeric_limits<double>::signaling_NaN());

template <typename T = double>
class Angle : public PhysicalQuantity<Angle<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Angle)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(radians, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(degrees, M_PI/180.0);

  static Angle<T> degMinMc(T deg, T min, T mc) {
      return Angle<T>::degrees(deg + (1.0/60)*(min + 0.001*mc));
  }
};

template <typename T = double>
class Length : public PhysicalQuantity<Length<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Length)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(meters, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(kilometers, 1000.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(nauticalMiles, 1852.0);
};

template <typename T = double>
class Velocity : public PhysicalQuantity<Velocity<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Velocity)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(metersPerSecond, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(knots, 1852/3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(kilometersPerHour, 1.0/3.6);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(milesPerHour, 0.44704);
};

template <typename T = double>
class Duration : public PhysicalQuantity<Duration<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Duration)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(seconds, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(minutes, 60.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(hours, 3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(days, 24*3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(weeks, 7*24*3600.0);
  std::string str() const {
    std::stringstream ss;
    Duration<T> remaining(*this);

#define FORMAT_DURATION_UNIT(unit) \
    if (remaining.unit() >= 1) { \
      if (ss.str().size() > 0) ss << ", "; \
      ss << floor(remaining.unit()) << " " #unit ; \
      remaining -= unit(floor(remaining.unit())); \
    }
    FORMAT_DURATION_UNIT(weeks)
    FORMAT_DURATION_UNIT(days)
    FORMAT_DURATION_UNIT(hours)
    FORMAT_DURATION_UNIT(minutes)
    FORMAT_DURATION_UNIT(seconds)
#undef FORMAT_DURATION_UNIT
    return ss.str();
  }
};

template <typename T = double>
class Mass : public PhysicalQuantity<Mass<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Mass)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(kilograms, 1.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(skeppund, 170.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(lispund, 170.0/20.0);
};

}

template <typename T>
T cos(sail::Angle<T> x) {return cos(x.radians());}

template <typename T>
T sin(sail::Angle<T> x) {return sin(x.radians());}

#undef MAKE_PHYSQUANT_TO_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_FROM_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_UNIT_CONVERTERS
#undef DECLARE_PHYSQUANT_CONSTRUCTORS


#endif /* PHYSICALQUANTITY_H_ */
