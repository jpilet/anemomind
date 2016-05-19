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
#include <type_traits>
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

#define FOREACH_TIME_UNIT(OP) \
    OP(Time, 0, milliseconds, 0.001) \
    OP(Time, 1, seconds, 1.0) \
    OP(Time, 2, minutes, 60.0) \
    OP(Time, 3, hours, 3600.0) \
    OP(Time, 4, days, 24*3600.0) \
    OP(Time, 5, weeks, 7*24*3600.0)

#define FOREACH_LENGTH_UNIT(OP) \
  OP(Length, 6, meters, 1.0) \
  OP(Length, 7, kilometers, 1000.0) \
  OP(Length, 8, nauticalMiles, 1852.0) // 1 nautical mile = 1852.0 m

#define FOREACH_ANGLE_UNIT(OP) \
  OP(Angle, 9, radians, 1.0) \
  OP(Angle, 10, degrees, M_PI/180.0) // 1 degree = M_PI/180 radians <=> 180 degrees = M_PI radians

#define FOREACH_VELOCITY_UNIT(OP) \
  OP(Velocity, 11, metersPerSecond, 1.0) \
  OP(Velocity, 12, knots, 1852.0/3600.0) \
  OP(Velocity, 13, kilometersPerHour, 1000.0/3600.0) \
  OP(Velocity, 14, milesPerHour, 1609.0/3600.0)

#define FOREACH_MASS_UNIT(OP) \
  OP(Mass, 15, kilograms, 1.0) \
  OP(Mass, 16, skeppund, 170.0) \
  OP(Mass, 17, lispund, 170.0/20)

#define FOREACH_UNIT(OP) \
  FOREACH_TIME_UNIT(OP) \
  FOREACH_LENGTH_UNIT(OP) \
  FOREACH_ANGLE_UNIT(OP) \
  FOREACH_VELOCITY_UNIT(OP) \
  FOREACH_MASS_UNIT(OP)

template <int Index>
struct IntToUnitFactor {};

#define MAKE_INT_TO_UNIT_FACTOR(type, index, name, factor) \
  template<> struct IntToUnitFactor<index> { \
  static constexpr double get() {return factor;} };
FOREACH_UNIT(MAKE_INT_TO_UNIT_FACTOR)
#undef MAKE_INT_TO_UNIT_FACTOR

template <typename T, int FromUnit, int ToUnit>
struct ConvertUnit {
public:
  static T apply(T x) {
    constexpr double f = IntToUnitFactor<ToUnit>::get()/IntToUnitFactor<FromUnit>::get();
    return T(f)*x;
  }
};

template <typename T, int Unit>
struct ConvertUnit<T, Unit, Unit> {
public:
  static T apply(T x) {return x;}
};

/*template <int TimeBase, int LengthBase, int VelocityBase, int AngleBase, int MassBase>
struct GeneralUnitSystem {
#define MAKE_WRAPPERS(base, )
};*/

/*namespace UnitSystem {

  struct SI {
    static const int timeBase = 1;
    static const int lengthBase = 6;
    static const int velocityBase = 11;
    static const int
  };

};*/

namespace UnitSystem {
  struct SI {
    static const bool mixable = true;
    static const int Time = 1;
    static const int Length = 6;
    static const int Angle = 9;
    static const int Velocity = 11;
    static const int Mass = 15;
  };

  struct AnemoOld {
    static const bool mixable = false;
    static const int Time = 0;
    static const int Length = 6;
    static const int Angle = 10;
    static const int Velocity = 12;
    static const int Mass = 15;
  };

  template <typename T, typename System>
  struct Compatible {
    static const bool value = true;
  };

  template <typename T>
  struct Compatible<T, SI> {
    static const bool value = std::is_floating_point<T>::value;
  };
};

template <typename T, typename system, int TimeDim/*t*/, int LengthDim/*l*/, int AngleDim/*a*/, int MassDim/*m*/>
class PhysicalQuantity;

#if ON_SERVER
template <typename T, typename sys>
std::string physQuantToString(const PhysicalQuantity<T, sys, 1, 0, 0, 0> &x);
#endif

template <typename T, typename System, int TimeDim/*t*/, int LengthDim/*l*/, int AngleDim/*a*/, int MassDim/*m*/>
class PhysicalQuantity {
public:
  static_assert(UnitSystem::Compatible<T, System>::value,
      "The type T is incompatible with the unit system");

  typedef T ValueType;

  T getScalar() const {
    return _x;
  }

  operator T () const {
    return this->getScalar();
  }

  PhysicalQuantity() : _x(NAN) {}

  static constexpr bool isScalar = (TimeDim == 0 && LengthDim == 0 && AngleDim == 0 && MassDim == 0);
  static constexpr bool isTime = (TimeDim == 1 && LengthDim == 0 && AngleDim == 0 && MassDim == 0);
  static constexpr bool isLength = (TimeDim == 0 && LengthDim == 1 && AngleDim == 0 && MassDim == 0);
  static constexpr bool isAngle = (TimeDim == 0 && LengthDim == 0 && AngleDim == 1 && MassDim == 0);
  static constexpr bool isMass = (TimeDim == 0 && LengthDim == 0 && AngleDim == 0 && MassDim == 1);
  static constexpr bool isVelocity = (TimeDim == -1 && LengthDim == 1 && AngleDim == 0 && MassDim == 1);

  typedef PhysicalQuantity<T, System, TimeDim, LengthDim, AngleDim, MassDim> ThisType;


#define MAKE_UNIT_CONVERTERS(type, index, name, factor) \
  static ThisType name(T x) { \
    return ThisType(ConvertUnit<T, index, System::type>::apply(x)); \
  } \
  T name() const { \
    return ConvertUnit<T, System::type, index>::apply(_x); \
  }
  FOREACH_UNIT(MAKE_UNIT_CONVERTERS)

  // Should not be used outside of this class/file.
  static ThisType pleaseAvoidThisPrivateConstructor(T x) {
    return ThisType(x);
  }

  static PhysicalQuantity<T, System, 0, 0, 0, 0> scalar(T x) {return PhysicalQuantity<T, System, 0, 0, 0, 0>(x);}

  static PhysicalQuantity<T, System, TimeDim, LengthDim, AngleDim, MassDim> wrap(T x) {
    return PhysicalQuantity<T, System, TimeDim, LengthDim, AngleDim, MassDim>(x);
  }

  template <int t, int l, int a, int m>
  PhysicalQuantity<T, System, TimeDim + t, LengthDim + l, AngleDim + a, MassDim + m> operator*(
      const PhysicalQuantity<T, System, t, l, a, m> &other) const {
    PhysicalQuantity<T, System, TimeDim + t, LengthDim + l,
    AngleDim + a, MassDim + m>::pleaseAvoidThisPrivateConstructor(_x*other._x);
  }

  template <int t, int l, int a, int m>
  PhysicalQuantity<T, System, TimeDim - t, LengthDim - l, AngleDim - a, MassDim - m> operator/(
      const PhysicalQuantity<T, System, t, l, a, m> &other) const {
    PhysicalQuantity<T, System, TimeDim - t, LengthDim - l,
    AngleDim - a, MassDim - m>::pleaseAvoidThisPrivateConstructor(_x/other._x);
  }

  ThisType operator*(T s) const {
    return ThisType(s*_x);
  }

  ThisType scaled(T s) const {
    return ThisType(s*_x);
  }

  static ThisType zero() {
    return ThisType(T(0.0));
  }

  ThisType operator+(const ThisType &other) const {
    return ThisType(_x + other._x);
  }

  ThisType operator-(const ThisType &other) const {
    return ThisType(_x - other._x);
  }

  ThisType &operator+=(const ThisType &other) {
    _x += other._x;
    return *this;
  }

  ThisType &operator-=(const ThisType &other) {
    _x -= other._x;
    return *this;
  }

  ThisType fabs() const { return ThisType(::fabs(_x)); }

  bool isNaNQuantity() const { return sail::isNaN(_x); }

  bool isFiniteQuantity() const { return sail::isFinite(_x); }

  ThisType operator-() const {
    return ThisType(-_x);
  }

  bool operator < (ThisType other) const {return _x < other._x;}
  bool operator <= (ThisType other) const {return _x <= other._x;}
  bool operator > (ThisType other) const {return _x > other._x;}
  bool operator >= (ThisType other) const {return _x >= other._x;}
  bool operator == (ThisType other) const {return _x == other._x;}


  template <typename S>
  PhysicalQuantity<S, System, TimeDim, LengthDim, AngleDim, MassDim> cast() const {
    return PhysicalQuantity<S, System,
        TimeDim, LengthDim,
        AngleDim, MassDim>::pleaseAvoidThisPrivateConstructor(
            static_cast<S>(_x));
  }

  template <typename S>
  operator PhysicalQuantity<S, System, TimeDim, LengthDim, AngleDim, MassDim>() const {
    return cast<S>();
  }

  ThisType directionDifference(const ThisType& other) const {
    static_assert(isAngle, "Only applicable to angles");
    return (*this - other).normalizedAt0();
  }

  ThisType moveToInterval(ThisType lower, ThisType upper) const {
    static_assert(isAngle, "Only applicable to angles");
    ThisType result(*this);
    while (result < lower) {
      result += ThisType::degrees(T(360));
    }
    while (result >= upper) {
      result -= ThisType::degrees(T(360));
    }
    return result;
  }

  ThisType normalizedAt0() const {
    static_assert(isAngle, "Only applicable to angles");
    return moveToInterval(ThisType::degrees(T(-180)),
        ThisType::degrees(T(180)));
  }

  ThisType positiveMinAngle() const {
    static_assert(isAngle, "Only applicable to angles");
    return moveToInterval(ThisType::degrees(T(0)),
        ThisType::degrees(T(360)));
  }

  static ThisType degMinMc(T deg, T min, T mc) {
    static_assert(isAngle, "Only applicable to angles");
    return ThisType::degrees(T(deg + (1.0/60)*(min + 0.001*mc)));
  }

  void sincos(T *sinAngle, T *cosAngle) const {
    static_assert(isAngle, "Only applicable to angles");
    T rad = radians();
    *sinAngle = sin(rad);
    *cosAngle = cos(rad);
  }

#ifdef ON_SERVER
  std::string str() const {
    return physQuantToString(*this);
  }

  // Special method returning true for the comparison nan == nan.
  bool eqWithNan(ThisType other) const {
    return strictEquality(_x, other._x);
  }

  bool nearWithNan(ThisType other, double marg) const {
    return sail::nearWithNan(_x, other._x, marg);
  }

#endif
private:
  PhysicalQuantity(T x) : _x(x) {}
  T _x;
};



// http://en.cppreference.com/w/cpp/language/type_alias
template <typename T=double, typename System=UnitSystem::SI>
using Duration = PhysicalQuantity<T, System, 1, 0, 0, 0>;

template <typename T=double, typename System=UnitSystem::SI>
using Length = PhysicalQuantity<T, System, 0, 1, 0, 0>;

template <typename T=double, typename System=UnitSystem::SI>
using Velocity = PhysicalQuantity<T, System, -1, 1, 0, 0>;

template <typename T=double, typename System=UnitSystem::SI>
using Angle = PhysicalQuantity<T, System, 0, 0, 1, 0>;

template <typename T=double, typename System=UnitSystem::SI>
using Mass = PhysicalQuantity<T, System, 0, 0, 0, 1>;

template <typename T, typename System, int t, int l, int a, int m>
PhysicalQuantity<T, System, t, l, a, m> operator*(T s,
    const PhysicalQuantity<T, System, t, l, a, m> &x) {
  return x*s;
}

#if ON_SERVER
template <typename T, typename System>
std::string physQuantToString(const PhysicalQuantity<T, System, 1, 0, 0, 0> &x) {
   std::stringstream ss;
   Duration<T> remaining(x);
#define FORMAT_DURATION_UNIT(unit) \
   if (remaining.unit() >= 1) { \
     if (ss.str().size() > 0) {ss << ", ";} \
     ss << floor(remaining.unit()) << " " #unit ; \
     remaining -= Duration<T>::unit(floor(remaining.unit())); \
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







/////////////////////////////////////////////////////// Extra stuff
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

template <typename T, typename s, int t, int l, int a, int m>
bool isFinite(const PhysicalQuantity<T, s, t, l, a, m> &x) {
  return x.isFiniteQuantity();
}
template <typename T, typename s, int t, int l, int a, int m>
bool isNaN(const PhysicalQuantity<T, s, t, l, a, m> &x) {
  return x.isNaNQuantity();
}

}  // namespace sail

#undef MAKE_PHYSQUANT_TO_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_FROM_UNIT_CONVERTER
#undef MAKE_PHYSQUANT_UNIT_CONVERTERS
#undef DECLARE_PHYSQUANT_CONSTRUCTORS

template <typename T, typename s, int t, int l, int a, int m>
sail::PhysicalQuantity<T, s, t, l, a, m> fabs(sail::PhysicalQuantity<T, s, t, l, a, m> x) {
  return x.fabs();
}

#endif /* PHYSICALQUANTITY_H_ */
