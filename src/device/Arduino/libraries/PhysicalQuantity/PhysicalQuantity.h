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

#include <server/common/numerics.h>
#ifdef ON_SERVER
#include <cmath>
#include <limits>
#include <server/common/math.h>
#include <sstream>
#include <string>
#else
#include <math.h>
#endif

// TODO: Do we still need this?
// workaround some arduino #define..
#ifdef degrees
#undef degrees
#endif
#ifdef radians
#undef radians
#endif

#ifdef isnan
#undef isnan
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
  ClassName<Other> cast() const { return ClassName<Other>::baseUnit(static_cast<Other>(this->baseUnit())); } \
  template<class Other> operator ClassName<Other>() const { return cast<Other>(); }

template <typename Quantity, typename Value>
class PhysicalQuantity {
 public:
#ifdef ON_SERVER
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

  static Quantity zero() {
    return Quantity::makeFromX(0.0);
  }

  Quantity scaled(Value s) const {
      return Quantity::makeFromX(_x * s);
  }

  // Syntactic sugar for 'scaled'
  Quantity operator* (Value other) const {
    return scaled(other);
  }

  Value operator/ (Quantity other) const {
    return _x/other._x;
  }


  Quantity fabs() const { return Quantity::makeFromX(::fabs(_x)); }

  // Please use sail::isFinite defined in
  // <server/common/numeric.h> to test if
  // PhysicalQuantity is finite. Same goes for nan.
  bool isNaNQuantity() const { return sail::isNaN(_x); }
  bool isFiniteQuantity() const { return sail::isFinite(_x); }

  // Comparison --> bool
  bool operator < (ThisQuantity other) const {return _x < other.get();}
  bool operator <= (ThisQuantity other) const {return _x <= other.get();}
  bool operator > (ThisQuantity other) const {return _x > other.get();}
  bool operator >= (ThisQuantity other) const {return _x >= other.get();}
  bool operator == (ThisQuantity other) const {return _x == other.get();}

#ifdef ON_SERVER
  // Special method returning true for the comparison nan == nan.
  bool eqWithNan(ThisQuantity other) const {
    return strictEquality(_x, other.get());
  }

  bool nearWithNan(ThisQuantity other, double marg) const {
    return sail::nearWithNan(_x, other.get(), marg);
  }
#endif
 protected:
  Value get() const {return _x;}
  PhysicalQuantity(Value x) : _x(x) {}
#ifdef ON_SERVER
  PhysicalQuantity() : _x(Quantity::defaultValue) {}
#else
  PhysicalQuantity() { }
#endif
  Value _x;
 private:
  static Quantity makeFromX(Value X) { return Quantity(PhysicalQuantity<Quantity, Value>(X)); }
};

#ifdef ON_SERVER
template <typename Quantity, typename Value>
const Value PhysicalQuantity<Quantity, Value>::defaultValue =
    Value(std::numeric_limits<double>::signaling_NaN());
#endif

template <typename T = double>
class Angle : public PhysicalQuantity<Angle<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Angle, degrees)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(radians, 180.0/M_PI);

  Angle directionDifference(const Angle<T>& other) const {
    return (*this - other).normalizedAt0();
  }

  Angle<T> moveToInterval(Angle<T> lower, Angle<T> upper) const {
    Angle<T> result(*this);
    while (result < lower) {
      result += Angle<T>::degrees(T(360));
    }
    while (result >= upper) {
      result -= Angle<T>::degrees(T(360));
    }
    return result;
  }

  Angle<T> normalizedAt0() const {
    return moveToInterval(Angle<T>::degrees(T(-180)),
        Angle<T>::degrees(T(180)));
  }

  Angle<T> positiveMinAngle() const {
    return moveToInterval(Angle<T>::degrees(T(0)),
        Angle<T>::degrees(T(360)));
  }

  static Angle<T> degMinMc(T deg, T min, T mc) {
      return Angle<T>::degrees(T(deg + (1.0/60)*(min + 0.001*mc)));
  }
  void sincos(T *sinAngle, T *cosAngle) const {
    T rad = radians();
    *sinAngle = sin(rad);
    *cosAngle = cos(rad);
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
  DECLARE_PHYSQUANT_CONSTRUCTORS(Velocity, knots)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(metersPerSecond, 3600.0/1852);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(kilometersPerHour, 1.0/1.852);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(milesPerHour, 0.868976242);
};

template <typename T = double>
class Duration : public PhysicalQuantity<Duration<T>, T> {
  DECLARE_PHYSQUANT_CONSTRUCTORS(Duration, milliseconds)
 public:
  MAKE_PHYSQUANT_UNIT_CONVERTERS(seconds, 1000);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(minutes, 60.0 * 1000.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(hours, 3600.0 * 1000.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(days, 24*3600.0 * 1000.0);
  MAKE_PHYSQUANT_UNIT_CONVERTERS(weeks, 7*24*3600.0 * 1000.0);
#ifdef ON_SERVER
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

template <typename Q, typename T>
PhysicalQuantity<Q, T> operator* (T s, PhysicalQuantity<Q, T> x) {
  return x*s;
}



template<typename T, int N>
class FixedArray {
 public:
  FixedArray() { }
  FixedArray(const FixedArray& a) {
    for (int i = 0; i < N; ++i) {
      _data[i] = a._data[i];
    }
  }

  T& operator[](int i) { return _data[i]; }
  const T& operator[](int i) const { return _data[i]; }

  const T* data() const { return _data; }
 private:
  T _data[N];
};

template <typename T, int N>
class Vectorize : public FixedArray<T, N> {
  public:
#ifdef ON_SERVER
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

    Vectorize<T, N> operator + (const Vectorize<T, N>& other) const {
        Vectorize result;
        for (int i = 0; i < N; ++i) {
            result[i] = (*this)[i] + other[i];
        }
        return result;
    }

    Vectorize<T, N> operator - (const Vectorize<T, N>& other) const {
        Vectorize result;
        for (int i = 0; i < N; ++i) {
            result[i] = (*this)[i] - other[i];
        }
        return result;
    }

    template <typename FactorType>
    Vectorize<T, N> scaled(FactorType factor) const {
        Vectorize result;
        for (int i = 0; i < N; ++i) {
            result[i] = (*this)[i].scaled(factor);
        }
        return result;
    }

    Vectorize<T, N> operator- () const {
      return Vectorize<T, N>{-(*this)[0], -(*this)[1]};
    }

    bool operator == (const Vectorize<T, N>& other) const {
        for (int i = 0; i < N; ++i) {
          if (!((*this)[i] == other[i])) return false;
        }
        return true;
    }

    Vectorize<T, N>& operator+= (const Vectorize<T, N>& other) {
      for (int i = 0; i < N; ++i) {
        (*this)[i] += other[i];
      }
      return *this;
    }

    Vectorize() { }
  private:
};

template <typename FactorType, typename ElemType, int N>
Vectorize<ElemType, N> operator*(FactorType x, const Vectorize<ElemType, N> &v) {
  return v.scaled(x);
}

}  // namespace sail

// Overloading sin and cos must be done outside namespace
template <typename T>
T cos(sail::Angle<T> x) {return cos(x.radians());}

template <typename T>
T sin(sail::Angle<T> x) {return sin(x.radians());}

template <typename T>
T tan(sail::Angle<T> x) {return tan(x.radians());}

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

    HorizontalMotion() { }

    static HorizontalMotion<T> zero() {
        return HorizontalMotion(BaseType::all(InnerType::knots(0)));
    }
    static HorizontalMotion<T> polar(Velocity<T> speed, Angle<T> direction) {
        // A direction of 0 points to north.
        T sinDir, cosDir;
        direction.sincos(&sinDir, &cosDir);
        return HorizontalMotion<T>(
            speed.scaled(sinDir),
            speed.scaled(cosDir));
    }

    Velocity<T> norm() const {
        T a = (*this)[0].knots();
        T b = (*this)[1].knots();
        return Velocity<T>::knots(sqrt(a*a + b*b));
    }
    Angle<T> angle() const {
        return Angle<T>::radians(atan2(
                (*this)[0].knots(),
                (*this)[1].knots()));
    }

    template <typename Dst>
    HorizontalMotion<Dst> cast() const {
      return HorizontalMotion<Dst>((*this)[0], (*this)[1]);
    }

    // Define what the vector dimensions mean.
    enum {
        EAST_TO_WEST = 0,
        SOUTH_TO_NORTH = 1
    };
};

template <typename Q, typename V>
bool IsFinite(const PhysicalQuantity<Q, V> &x) {
  return x.isFiniteQuantity();
}
template <typename Q, typename V>
bool IsNaN(const PhysicalQuantity<Q, V> &x) {
  return x.isNaNQuantity();
}

}  // namespace sail

#undef MAKE_PHYSQUANT_TO_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_FROM_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_UNIT_CONVERTERS
#undef DECLARE_PHYSQUANT_CONSTRUCTORS

template <typename Quantity, typename Value>
Quantity fabs(sail::PhysicalQuantity<Quantity, Value> x) {
  return x.fabs();
}

#endif /* PHYSICALQUANTITY_H_ */
