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

#ifndef ANEMOMIND_DEVICE
#include <cmath>
#include <limits>
#include <server/common/math.h>
#include <sstream>
#include <string>
#else
#include <math.h>
#endif

namespace sail {

/*
 * These macros may not be in conformity with the
 * coding style guide but they reduce code duplication effort
 * and make the code much more readable than it otherwise would be.
 */
#define MAKE_PHYSQUANT_UNIT_CONVERTERS(name, fromFactor) \
  T name() const {return (T(1.0) / T(fromFactor))*(this->_x);} \
  static ThisType name(T x) {return ThisType(T(fromFactor)*x);}

#define DECLARE_PHYSQUANT_CONSTRUCTORS(ClassName, baseUnit) \
  private: \
    ClassName(T x) : PhysicalQuantity<ClassName<T>, T>(x) {} \
  public: \
    typedef ClassName<T> ThisType; \
    ClassName() : PhysicalQuantity<ClassName<T>, T>() { } \
    ClassName(const PhysicalQuantity<ClassName<T>, T> &x) : PhysicalQuantity<ClassName<T>, T>(x) {} \
  T baseUnit() const { return this->_x; } \
  static ThisType baseUnit(T x) { return ThisType(x); } \
  template<class Other> \
  ClassName<Other> cast() const { return ClassName<Other>::baseUnit(Other(this->baseUnit())); }

template <typename Quantity, typename Value>
class PhysicalQuantity {
 public:
#ifndef ANEMOMIND_DEVICE
  const static Value defaultValue;
#endif
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

  Quantity operator-() const {
    return Quantity::makeFromX(-_x);
  }

  ThisQuantity &operator -= (ThisQuantity other) {
    _x -= other.get();
    return *this;
  }

  Quantity scaled(Value s) const {
      return Quantity::makeFromX(_x * s);
  }

  // Comparison --> bool
  bool operator < (ThisQuantity other) const {return _x < other.get();}
  bool operator <= (ThisQuantity other) const {return _x <= other.get();}
  bool operator > (ThisQuantity other) const {return _x > other.get();}
  bool operator >= (ThisQuantity other) const {return _x >= other.get();}
  bool operator == (ThisQuantity other) const {return _x == other.get();}

 protected:
  Value get() const {return _x;}
  PhysicalQuantity(Value x) : _x(x) {}
#ifndef ANEMOMIND_DEVICE
  PhysicalQuantity() : _x(Quantity::defaultValue) {}
#else
  PhysicalQuantity() { }
#endif
  Value _x;
 private:
  static Quantity makeFromX(Value X) { return Quantity(PhysicalQuantity<Quantity, Value>(X)); }
};

#ifndef ANEMOMIND_DEVICE
template <typename Quantity, typename Value>
const Value PhysicalQuantity<Quantity, Value>::defaultValue =
    Value(std::numeric_limits<double>::signaling_NaN());
#endif

template <typename T = double>
class Angle : public PhysicalQuantity<Angle<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Angle, radians)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(degrees, M_PI/180.0);

  Angle directionDifference(const Angle<T>& other) const {
    return radians(normalizeAngleBetweenMinusPiAndPi(
            this->get() - other.get()));
  }

  static Angle<T> degMinMc(T deg, T min, T mc) {
      return Angle<T>::degrees(deg + (1.0/60)*(min + 0.001*mc));
  }
};

template <typename T = double>
class Length : public PhysicalQuantity<Length<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Length, meters)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(kilometers, 1000.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(nauticalMiles, 1852.0);
};

template <typename T = double>
class Velocity : public PhysicalQuantity<Velocity<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Velocity, metersPerSecond)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(knots, 1852/3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(kilometersPerHour, 1.0/3.6);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(milesPerHour, 0.44704);
};

template <typename T = double>
class Duration : public PhysicalQuantity<Duration<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Duration, seconds)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(minutes, 60.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(hours, 3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(days, 24*3600.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(weeks, 7*24*3600.0);
#ifndef ANEMOMIND_DEVICE
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
#endif
};

template <typename T = double>
class Mass : public PhysicalQuantity<Mass<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Mass, kilograms)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(skeppund, 170.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(lispund, 170.0/20.0);

};

template<typename T, int N>
class FixedArray {
 public:
  FixedArray();
  FixedArray(const FixedArray& a) {
    for (int i = 0; i < N; ++i) {
      _data[i] = a._data[i];
    }
  }

  T& operator[](int i) { return _data[i]; }
  const T& operator[](int i) const { return _data[i]; }

 private:
  T _data[N];
};

template <typename T, int N>
class Vectorize : public FixedArray<T, N> {
  public:
#ifndef ANEMOMIND_DEVICE
    Vectorize<T, N>(std::initializer_list<T> list) {
        int i=0;
        for (T element : list) {
            (*this)[i++] = element;
        }
    }
#endif

    explicit Vectorize<T, N>(const T x[N]) {
      for (int i = 0; i < N; i++) {
        (*this)[i] = x[i];
      }
    }

    static Vectorize<T, N> all(T value) {
        Vectorize<T, N> result;
        for (int i = 0; i < N; ++i) {
            result[i] = value;
        }
        return result;
    }

    Vectorize<T, N> operator + (const Vectorize<T, N>& other) {
        Vectorize result;
        for (int i = 0; i < N; ++i) {
            result[i] = (*this)[i] + other[i];
        }
        return result;
    }

    Vectorize<T, N> operator - (const Vectorize<T, N>& other) {
        Vectorize result;
        for (int i = 0; i < N; ++i) {
            result[i] = (*this)[i] - other[i];
        }
        return result;
    }

    Vectorize<T, N> scaled(double factor) {
        Vectorize result;
        for (int i = 0; i < N; ++i) {
            result[i] = (*this)[i].scaled(factor);
        }
        return result;
    }

    Vectorize() { }
  private:
};

template <typename FactorType, typename ElemType, int N>
Vectorize<ElemType, N> operator*(FactorType x, const Vectorize<ElemType, N> &v) {
  return v*x;
}

}  // namespace sail

// Overloading sin and cos must be done outside namespace
template <typename T>
T cos(sail::Angle<T> x) {return cos(x.radians());}

template <typename T>
T sin(sail::Angle<T> x) {return sin(x.radians());}

namespace sail {

template <typename T>
class HorizontalMotion : public Vectorize<Velocity<T>, 2> {
  public:
    typedef Velocity<T> InnerType;
    typedef Vectorize<Velocity<T>, 2> BaseType;

    HorizontalMotion(InnerType eastWest, InnerType southNorth) {
      (*this)[0] = eastWest;
      (*this)[1] = southNorth;
    }

    HorizontalMotion(const BaseType& base) : BaseType(base) { }

    static HorizontalMotion<T> zero() {
        return HorizontalMotion(BaseType::all(InnerType::metersPerSecond(0))); }
    static HorizontalMotion<T> polar(Velocity<T> speed, Angle<T> direction) {
        // A direction of 0 points to north.
        return HorizontalMotion<T>(
            speed.scaled(sin(direction)),
            speed.scaled(cos(direction)));
    }

    Velocity<T> norm() const {
        T a = (*this)[0].metersPerSecond();
        T b = (*this)[1].metersPerSecond();
        return Velocity<T>::metersPerSecond(sqrt(a*a + b*b));
    }
    Angle<T> angle() const {
        return Angle<T>::radians(atan2(
                (*this)[0].metersPerSecond(),
                (*this)[1].metersPerSecond()));
    }

    // Define what the vector dimensions mean.
    enum {
        EAST_TO_WEST = 0,
        SOUTH_TO_NORTH = 1
    };
};

}

#undef MAKE_PHYSQUANT_TO_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_FROM_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_UNIT_CONVERTERS
#undef DECLARE_PHYSQUANT_CONSTRUCTORS


#endif /* PHYSICALQUANTITY_H_ */
