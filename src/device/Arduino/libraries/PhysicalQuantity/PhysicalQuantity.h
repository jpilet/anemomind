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
#include <server/common/PositiveMod.h>
#include <server/common/Optional.h>
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

template <typename Function, typename T>
struct FunctionReturnType {
  typedef decltype(std::declval<Function>()(std::declval<T>())) type;
};

template <typename A, typename B>
using QuantityPerQuantity = decltype(std::declval<A>()/std::declval<B>());

template <typename A, typename B>
using QuantityByQuantity = decltype(std::declval<A>()*std::declval<B>());

template <typename T>
using QuantitySquared = QuantityByQuantity<T, T>;

// For instance, we could define Acceleration as
// typedef Per<Velocity<double>, Duration<double>> Acceleration;


// OP(type, name, factor)
#define FOREACH_TIME_UNIT(OP) \
    OP(Time, milliseconds, 0.001) \
    OP(Time, seconds, 1.0) \
    OP(Time, minutes, 60.0) \
    OP(Time, hours, 3600.0) \
    OP(Time, days, 24*3600.0) \
    OP(Time, weeks, 7*24*3600.0)

#define FOREACH_LENGTH_UNIT(OP) \
  OP(Length, meters, 1.0) \
  OP(Length, kilometers, 1000.0) \
  OP(Length, nauticalMiles, 1852.0) // 1 nautical mile = 1852.0 m

#define FOREACH_ANGLE_UNIT(OP) \
  OP(Angle, radians, 1.0) \
  OP(Angle, degrees, M_PI/180.0) // 1 degree = M_PI/180 radians <=> 180 degrees = M_PI radians

#define FOREACH_VELOCITY_UNIT(OP) \
  OP(Velocity, metersPerSecond, 1.0) \
  OP(Velocity, knots, 1852.0/3600.0) \
  OP(Velocity, kilometersPerHour, 1000.0/3600.0) \
  OP(Velocity, milesPerHour, 1609.0/3600.0)

#define FOREACH_MASS_UNIT(OP) \
  OP(Mass, kilograms, 1.0) \
  OP(Mass, skeppund, 170.0) \
  OP(Mass, lispund, 170.0/20)

#define FOREACH_UNIT(OP) \
  FOREACH_TIME_UNIT(OP) \
  FOREACH_LENGTH_UNIT(OP) \
  FOREACH_ANGLE_UNIT(OP) \
  FOREACH_VELOCITY_UNIT(OP) \
  FOREACH_MASS_UNIT(OP)

#define FOREACH_QUANTITY(OP) \
  OP(Time, seconds, 1, 0, 0, 0) \
  OP(Length, meters, 0, 1, 0, 0) \
  OP(Angle, radians, 0, 0, 1, 0) \
  OP(Mass, kilograms, 0, 0, 0, 1) \
  OP(Velocity, metersPerSecond, -1, 1, 0, 0)

enum class Quantity {
  // Any quantity that has not been declared maps to this one.
  Undeclared,
#define MAKE_QUANTITY_ENUM(name, siUnit, t, l, a, m) \
    name,
FOREACH_QUANTITY(MAKE_QUANTITY_ENUM)
#undef MAKE_QUANTITY_ENUM
};

enum class Unit {
  // This unit can be used with any quantity and is
  // the SI unit of that quantity. So even if we are working
  // with volume and there are no volume quantities declared,
  // this unit will in that case represents cubic meter.
  SIUnit,
#define MAKE_UNIT_ENUM(type, name, factor) \
  name,
FOREACH_UNIT(MAKE_UNIT_ENUM)
#undef MAKE_UNIT_ENUM
};

/*
 * This general template will, in particular, apply to
 * to Unit::SIUnit, which is the default unit for all
 * quantities that we have not declared, such as acceleration.
 */
template <Unit i>
struct UnitInfo {
  static constexpr double getFactor() {return 1.0;}
  static const Quantity quantity = Quantity::Undeclared;
  static const Unit unit = i;
};

// Here, the above template is being specialized for all declared units.
#define MAKE_INT_TO_UNIT_FACTOR(type, name, factor) \
  template<> struct UnitInfo<Unit::name> { \
  static constexpr double getFactor() {return factor;} \
  static const Quantity quantity = Quantity::type; \
  static const Unit unit = Unit::name; \
};
FOREACH_UNIT(MAKE_INT_TO_UNIT_FACTOR)
#undef MAKE_INT_TO_UNIT_FACTOR

/*
 * Default template that catches all quantities
 * that we have not declared, but that are nevertheless
 * valid, such as acceleration.
 */
template <typename unitsys, int t, int l, int a, int m>
struct QuantityInfo {
  static const Quantity quantity = Quantity::Undeclared;
  static const Unit unit = Unit::SIUnit;
};

#define MAKE_QUANTITY_INFO(name, siUnit, t, l, a, m) \
  template <typename unitsys> \
  struct QuantityInfo<unitsys, t, l, a, m> { \
    static const Unit unit = unitsys::name; \
    static const Quantity quantity = Quantity::name; \
  };
FOREACH_QUANTITY(MAKE_QUANTITY_INFO)
#undef MAKE_QUANTITY_INFO


// Converts between arbitrary units. This code is used
// by PhysicalQuantity.
template <typename T, Unit FromUnit, Unit ToUnit>
struct ConvertUnit {
  static_assert(
      UnitInfo<FromUnit>::quantity == UnitInfo<ToUnit>::quantity
      || FromUnit == Unit::SIUnit || ToUnit == Unit::SIUnit,
      "Incompatible quantitites");
  static T apply(T x) {
    constexpr double f = UnitInfo<FromUnit>::getFactor()/UnitInfo<ToUnit>::getFactor();
    return T(f)*x;
  }
};
// For the special case when the unit is the same,
// there is no need to multiply.
template <typename T, Unit u>
struct ConvertUnit<T, u, u> {
public:
  static T apply(T x) {return x;}
};

namespace UnitSystem {
  struct SI {
#define MAKE_SI_UNIT(name, siUnit, t, l, a, m) \
  static const Unit name = Unit::siUnit;
FOREACH_QUANTITY(MAKE_SI_UNIT)
#undef MAKE_SI_UNIT
  };

  // Suitable for integer and fixed-point representation
  struct CustomAnemoUnits : public SI {
    static const Unit Time = Unit::milliseconds;
    static const Unit Angle = Unit::degrees;
    static const Unit Velocity = Unit::knots;
  };
};

template <typename T, typename system, int TimeDim/*t*/, int LengthDim/*l*/, int AngleDim/*a*/, int MassDim/*m*/>
class PhysicalQuantity;

#if ON_SERVER
template <typename T, typename sys>
std::string physQuantToString(const PhysicalQuantity<T, sys, 1, 0, 0, 0> &x);
#endif

struct NonsenseType {};

template <typename T, typename System, int t, int l, int a, int m>
struct DimensionlessTraits {
  typedef NonsenseType PublicType;
  typedef T PrivateType;
  static NonsenseType get(T x) {return NonsenseType{};}
};

template <typename T, typename System>
struct DimensionlessTraits<T, System, 0, 0, 0, 0> {
  typedef NonsenseType PrivateType;
  typedef T PublicType;
  static T get(T x) {return x;}
};


template <typename T, typename Enable = void>
struct InitialValue {
  static T get() {return T();}
};

template <typename T>
struct InitialValue<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
  static T get() {return std::numeric_limits<T>::signaling_NaN();}
};

template <typename T>
struct InitialValue<T,
  typename std::enable_if<std::is_integral<T>::value>::type> {
  static T get() {return T(0);}
};


template <typename T, typename System, int TimeDim/*t*/, int LengthDim/*l*/, int AngleDim/*a*/, int MassDim/*m*/>
class PhysicalQuantity {
public:
  static const int valueDimension = 1; // How many elements that are needed to represent this object as vector.
  typedef DimensionlessTraits<T, System, TimeDim, LengthDim, AngleDim, MassDim> DimensionlessInfo;

  typedef T ValueType;

#if ON_SERVER
  PhysicalQuantity() : _x(InitialValue<T>::get()) {}
#else
  PhysicalQuantity() : _x() {}
#endif
  typedef QuantityInfo<System, TimeDim, LengthDim, AngleDim, MassDim> QInfo;
  typedef UnitInfo<QInfo::unit> UInfo;

  typedef PhysicalQuantity<T, System, TimeDim, LengthDim, AngleDim, MassDim> ThisType;

  static constexpr bool isDimensionless = TimeDim == 0 && LengthDim == 0
      && AngleDim == 0 && MassDim == 0;

#define MAKE_UNIT_CONVERTERS(type, name, factor) \
  static ThisType name(T x) { \
    static_assert(UnitInfo<Unit::name>::quantity == QInfo::quantity, "Incompatible unit and quantity"); \
    return ThisType(ConvertUnit<T, Unit::name, System::type>::apply(x)); \
  } \
  T name() const { \
    static_assert(UnitInfo<Unit::name>::quantity == QInfo::quantity, "Incompatible unit and quantity"); \
    return ConvertUnit<T, System::type, Unit::name>::apply(_x); \
  }
  FOREACH_UNIT(MAKE_UNIT_CONVERTERS)

  template <Unit unit>
  static ThisType make(T x) {
    static_assert(UnitInfo<unit>::quantity == QInfo::quantity
        || unit == Unit::SIUnit,
        "Incompatible unit and quantity");
    return ThisType(ConvertUnit<T, unit, UInfo::unit>::apply(x));
  }

  static ThisType makeFromSI(T x) {
    return make<Unit::SIUnit>(x);
  }

  template <Unit unit>
  T get() const {
    static_assert(UnitInfo<unit>::quantity == QInfo::quantity
        || unit == Unit::SIUnit,
        "Incompatible unit and quantity");
    return ConvertUnit<T, UInfo::unit, unit>::apply(_x);
  }

  T getSI() const {
    return get<Unit::SIUnit>();
  }

  static PhysicalQuantity<T, System, TimeDim, LengthDim, AngleDim, MassDim> wrap(T x) {
    return PhysicalQuantity<T, System, TimeDim, LengthDim, AngleDim, MassDim>(x);
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

  T dimensionless() const {
    static_assert(isDimensionless, "Only applicable to Dimensionlesss");
    return ConvertUnit<T, UInfo::unit, Unit::SIUnit>::apply(_x);
  }

  static ThisType dimensionless(T x) {
    static_assert(isDimensionless, "Only applicable to Dimensionless types");
    return ThisType(ConvertUnit<T, Unit::SIUnit, UInfo::unit>::apply(x));
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


  template <typename Function>
  PhysicalQuantity<typename FunctionReturnType<Function, T>::type,
      System, TimeDim, LengthDim, AngleDim, MassDim> mapObjectValues(Function f) const {
    typedef PhysicalQuantity<typename FunctionReturnType<Function, T>::type,
        System, TimeDim, LengthDim, AngleDim, MassDim> DstType;
    static constexpr Unit unit = DstType::UInfo::unit;
    return DstType::template make<unit>(f(get<unit>()));
  }

  template <typename S>
  PhysicalQuantity<S, System, TimeDim, LengthDim, AngleDim, MassDim> cast() const {
    typedef PhysicalQuantity<S, System,
            TimeDim, LengthDim,
            AngleDim, MassDim> DstType;
    static constexpr Unit unit = DstType::UInfo::unit;
    return DstType::template make<unit>(
            static_cast<S>(get<unit>()));
  }

  template <typename S> // TODO: Maybe allow to cast between different systems.
  operator PhysicalQuantity<S, System, TimeDim, LengthDim, AngleDim, MassDim>() const {
    return cast<S>();
  }

  ThisType directionDifference(const ThisType& other) const {
    static_assert(UInfo::quantity == Quantity::Angle, "Only applicable to angles");
    return (*this - other).normalizedAt0();
  }

  ThisType minimizeCyclicallyButNotLessThan(ThisType lower) const {
    static_assert(UInfo::quantity == Quantity::Angle, "Only applicable to angles");
    return positiveMod<ThisType>(*this - lower,
        ThisType::degrees(T(360))) + lower;
  }

  ThisType normalizedAt0() const {
    static_assert(UInfo::quantity == Quantity::Angle, "Only applicable to angles");
    return minimizeCyclicallyButNotLessThan(ThisType::degrees(T(-180)));
  }

  ThisType positiveMinAngle() const {
    static_assert(UInfo::quantity == Quantity::Angle, "Only applicable to angles");
    return minimizeCyclicallyButNotLessThan(ThisType::degrees(T(0)));
  }

  static ThisType degMinMc(T deg, T min, T mc) {
    static_assert(UInfo::quantity == Quantity::Angle, "Only applicable to angles");
    return ThisType::degrees(T(deg + (1.0/60)*(min + 0.001*mc)));
  }

  void sincos(T *sinAngle, T *cosAngle) const {
    static_assert(UInfo::quantity == Quantity::Angle, "Only applicable to angles");
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

  operator typename DimensionlessInfo::PublicType() const {
    return DimensionlessInfo::get(_x);
  }
private:
  PhysicalQuantity(T x) : _x(x) {}
  T _x;
};

template <typename T, typename sys,
  int t0, int l0, int a0, int m0,
  int t1, int l1, int a1, int m1>
struct Division {
  static PhysicalQuantity<T, sys, t0 - t1, l0 - l1, a0 - a1, m0 - m1>
    apply(const PhysicalQuantity<T, sys, t0, l0, a0, m0> &a,
          const PhysicalQuantity<T, sys, t1, l1, a1, m1> &b) {
    T aValue = a.getSI();
    T bValue = b.getSI();
    typedef PhysicalQuantity<T, sys, t0 - t1, l0 - l1, a0 - a1, m0 - m1> DstType;
    return DstType::makeFromSI(aValue/bValue);
  }
};

template <typename T, typename sys,
  int t, int l, int a, int m>
struct Division<T, sys, t, l, a, m, t, l, a, m> {
  static PhysicalQuantity<T, sys, 0, 0, 0, 0>
    apply(const PhysicalQuantity<T, sys, t, l, a, m> &A,
          const PhysicalQuantity<T, sys, t, l, a, m> &B) {
    /*
     *  We have some code out there that divides
     *  two quantities like this, where T is fixed-point
     *  or integer. So this specialization avoids the unnecessary
     *  loss of precision due to first converting it to an SI unit.
     */
    static const Unit unit = PhysicalQuantity<T, sys, t, l, a, m>::UInfo::unit;
    T aValue = A.template get<unit>();
    T bValue = B.template get<unit>();
    typedef PhysicalQuantity<T, sys, 0, 0, 0, 0> DstType;
    return DstType::dimensionless(aValue/bValue);
  }
};

template <typename T, typename sys,
  int t0, int l0, int a0, int m0,
  int t1, int l1, int a1, int m1>
PhysicalQuantity<T, sys, t0 - t1, l0 - l1, a0 - a1, m0 - m1> operator/(
    const PhysicalQuantity<T, sys, t0, l0, a0, m0> &a,
    const PhysicalQuantity<T, sys, t1, l1, a1, m1> &b) {
  return Division<T, sys, t0, l0, a0, m0, t1, l1, a1, m1>::apply(a, b);
}



template <typename T, typename sys,
  int t0, int l0, int a0, int m0,
  int t1, int l1, int a1, int m1>
static PhysicalQuantity<T, sys, t0 + t1, l0 + l1, a0 + a1, m0 + m1>
  operator*(const PhysicalQuantity<T, sys, t0, l0, a0, m0> &a,
            const PhysicalQuantity<T, sys, t1, l1, a1, m1> &b) {
  T aValue = a.getSI();
  T bValue = b.getSI();
  typedef PhysicalQuantity<T, sys, t0 + t1, l0 + l1, a0 + a1, m0 + m1> DstType;
  return DstType::makeFromSI(aValue*bValue);
}

// http://en.cppreference.com/w/cpp/language/type_alias
template <typename T=double, typename System=UnitSystem::CustomAnemoUnits>
using Dimensionless = PhysicalQuantity<T, System, 0, 0, 0, 0>;

template <typename T=double, typename System=UnitSystem::CustomAnemoUnits>
using Duration = PhysicalQuantity<T, System, 1, 0, 0, 0>;

template <typename T=double, typename System=UnitSystem::CustomAnemoUnits>
using Length = PhysicalQuantity<T, System, 0, 1, 0, 0>;

template <typename T=double, typename System=UnitSystem::CustomAnemoUnits>
using Velocity = PhysicalQuantity<T, System, -1, 1, 0, 0>;

template <typename T=double, typename System=UnitSystem::CustomAnemoUnits>
using Angle = PhysicalQuantity<T, System, 0, 0, 1, 0>;

template <typename T=double, typename System=UnitSystem::CustomAnemoUnits>
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
Angle<T> headingAngle(T x, T y) {
  return Angle<T>::radians(atan2(x, y));
}

template <typename T>
class HorizontalMotion : public Vectorize<Velocity<T>, 2> {
  public:
    typedef Velocity<T> InnerType;
    typedef Vectorize<Velocity<T>, 2> BaseType;
    static const int valueDimension = 2*Velocity<T>::valueDimension;
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
        return headingAngle<T>(
                (*this)[0].knots(),
                (*this)[1].knots());
    }

    Optional<Angle<T>> optionalAngle() const {
      auto alpha = angle();
      return isFinite(alpha)? alpha : Optional<Angle<T>>();
    }

    template <typename Dst>
    HorizontalMotion<Dst> cast() const {
      return HorizontalMotion<Dst>((*this)[0], (*this)[1]);
    }

    template <typename Function>
    HorizontalMotion<typename FunctionReturnType<Function, T>::type>
      mapObjectValues(Function f) const {
      return HorizontalMotion<typename FunctionReturnType<Function, T>::type>(
          (*this)[0].template mapObjectValues<Function>(f),
          (*this)[1].template mapObjectValues<Function>(f));
    }

    HorizontalMotion<T> rotate(Angle<T> angle) const {
      auto cosa = cos(angle);
      auto sina = sin(angle);
      return HorizontalMotion<T>{
        cosa*(*this)[0] + sina*(*this)[1],
        -sina*(*this)[0] + cosa*(*this)[1]
      };
    }

    // Define what the vector dimensions mean.
    enum {
        EAST_TO_WEST = 0,
        SOUTH_TO_NORTH = 1
    };
};

template <typename T>
Angle<T> interpolate(T lambda, Angle<T> a, Angle<T> b) {
  return a + lambda*(b - a).normalizedAt0();
}

template <typename T, typename s, int t, int l, int a, int m>
bool isFinite(const PhysicalQuantity<T, s, t, l, a, m> &x) {
  return x.isFiniteQuantity();
}
template <typename T>
bool isFinite(const HorizontalMotion<T> &x) {
  return isFinite(x[0]) && isFinite(x[1]);
}

template <typename T, typename s, int t, int l, int a, int m>
bool isNaN(const PhysicalQuantity<T, s, t, l, a, m> &x) {
  return x.isNaNQuantity();
}

// Literals
#define DEFINE_LITERAL(QUANTITY, WHAT, LIT) \
  inline QUANTITY<double> operator"" LIT (long double x) { \
    return QUANTITY<double>::WHAT(x); \
}
DEFINE_LITERAL(Angle, degrees, _deg)
DEFINE_LITERAL(Angle, radians, _rad)
DEFINE_LITERAL(Length, meters, _m)
DEFINE_LITERAL(Length, kilometers, _km)
DEFINE_LITERAL(Length, nauticalMiles, _M)
DEFINE_LITERAL(Duration, seconds, _s)
DEFINE_LITERAL(Duration, hours, _h)
DEFINE_LITERAL(Duration, hours, _hours)
DEFINE_LITERAL(Duration, minutes, _minutes)
DEFINE_LITERAL(Duration, minutes, _min)
DEFINE_LITERAL(Velocity, metersPerSecond, _mps)
DEFINE_LITERAL(Velocity, knots, _kn)
DEFINE_LITERAL(Velocity, knots, _kt)
#undef DEFINE_LITERAL


}  // namespace sail

template <typename T, typename s, int t, int l, int a, int m>
sail::PhysicalQuantity<T, s, t, l, a, m> fabs(sail::PhysicalQuantity<T, s, t, l, a, m> x) {
  return x.fabs();
}

#endif /* PHYSICALQUANTITY_H_ */
