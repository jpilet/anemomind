#ifndef NAUTICAL_ABSOLUTE_ORIENTATION_H
#define NAUTICAL_ABSOLUTE_ORIENTATION_H

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

template <typename T>
struct TypedAbsoluteOrientation {
  Angle<T> heading; // Where the boat is heading on the surface
  Angle<T> roll;
  Angle<T> pitch;

  static TypedAbsoluteOrientation<T> onlyHeading(Angle<T> x) {
    return TypedAbsoluteOrientation<T>{
      x,
      Angle<T>::degrees(T(0.0)),
      Angle<T>::degrees(T(0.0))
    };
  }

  bool operator==(const
      TypedAbsoluteOrientation<T> &other) const {
    return heading == other.heading
        && roll == other.roll
        && pitch == other.pitch;
  }

  template <typename Function>
  TypedAbsoluteOrientation<typename FunctionReturnType<Function, T>::type>
    mapObjectValues(Function f) {
    return TypedAbsoluteOrientation<T>{
      heading.template mapObjectValues<Function>(f),
      roll.template mapObjectValues<Function>(f),
      pitch.template mapObjectValues<Function>(f)
    };
  }
};

template <typename T>
TypedAbsoluteOrientation<T> interpolate(
    T lambda,
    const TypedAbsoluteOrientation<T> &a,
    const TypedAbsoluteOrientation<T> &b) {
  return TypedAbsoluteOrientation<T>{
    interpolate<T>(lambda, a.heading, b.heading),
    interpolate<T>(lambda, a.roll, b.roll),
    interpolate<T>(lambda, a.pitch, b.pitch)
  };
}

typedef TypedAbsoluteOrientation<double> AbsoluteOrientation;

template <typename T>
bool isFinite(const TypedAbsoluteOrientation<T> &x) {
  return isFinite(x.heading) && isFinite(x.pitch)
      && isFinite(x.roll);
}

}  // namespace sail

#endif  // NAUTICAL_ABSOLUTE_ORIENTATION_H
