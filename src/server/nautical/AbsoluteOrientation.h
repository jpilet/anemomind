#ifndef NAUTICAL_ABSOLUTE_ORIENTATION_H
#define NAUTICAL_ABSOLUTE_ORIENTATION_H

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <Eigen/Dense>

namespace sail {

/*
 * NOTE: If we are going to work with AbsoluteOrientation, we
 * need to establish a standard for how it maps to a rotation matrix.
 * Currently, it serves as a container to hold the output of the
 * BNO055 IMU. Then such a value can be mapped to a rotation using
 * BNO055AnglesToRotation. But another rotation sensor might use a
 * different coding of rotations. This is a potential source of
 * bugs and confusion.
 *
 * One solultion would be to introduce an additional template parameter to
 * TypedAbsoluteOrientation that is a value of some enum OrientCoding,
 * that tags the type with information of how it maps to a rotation.
 * And then we could have one such enum value for the BNO055 sensor.
 * That is what the log files currently store.
 *
 * As pointed out here,
 * https://eigen.tuxfamily.org/dox/group__TutorialGeometry.html
 * there are 24 different conventions...
 */
enum class OrientCoding {
  BNO055,
  Boat
};

template <typename T, OrientCoding c>
struct TypedAbsoluteOrientation {
  Angle<T> heading; // Where the boat is heading on the surface
  Angle<T> roll;
  Angle<T> pitch;
  typedef TypedAbsoluteOrientation<T, c> ThisType;
  static const int valueDimension = 3*Angle<T>::valueDimension;

  static ThisType onlyHeading(Angle<T> x) {
    return ThisType{
      x,
      Angle<T>::degrees(T(0.0)),
      Angle<T>::degrees(T(0.0))
    };
  }

  bool operator==(const
      ThisType &other) const {
    return heading == other.heading
        && roll == other.roll
        && pitch == other.pitch;
  }

  template <typename Function>
  TypedAbsoluteOrientation<
    typename FunctionReturnType<Function, T>::type, c>
    mapObjectValues(Function f) const {
    return TypedAbsoluteOrientation<
        typename FunctionReturnType<Function, T>::type, c>{
      heading.template mapObjectValues<Function>(f),
      roll.template mapObjectValues<Function>(f),
      pitch.template mapObjectValues<Function>(f)
    };
  }
};

template <typename T, OrientCoding c>
TypedAbsoluteOrientation<T, c> interpolate(
    T lambda,
    const TypedAbsoluteOrientation<T, c> &a,
    const TypedAbsoluteOrientation<T, c> &b) {
  return TypedAbsoluteOrientation<T, c>{
    interpolate<T>(lambda, a.heading, b.heading),
    interpolate<T>(lambda, a.roll, b.roll),
    interpolate<T>(lambda, a.pitch, b.pitch)
  };
}

// This is what we have been using so far to store orientations
// in the log files.
typedef TypedAbsoluteOrientation<double, OrientCoding::BNO055>
  AbsoluteOrientation;

template <typename T, OrientCoding c>
bool isFinite(const TypedAbsoluteOrientation<T, c> &x) {
  return isFinite(x.heading) && isFinite(x.pitch)
      && isFinite(x.roll);
}

Eigen::Matrix3d BNO055AnglesToRotation(
    const AbsoluteOrientation &orientation);

template <typename T>
using AbsoluteBoatOrientation
    = TypedAbsoluteOrientation<T, OrientCoding::Boat>;

template <typename T>
Eigen::Matrix<T, 3, 3> headingMatrix(Angle<T> h) {
  Eigen::Matrix<T, 3, 3> H;
  H << cos(h), sin(h), 0,
       -sin(h), cos(h), 0,
       0, 0, 1;
  return H;
}

template <typename T>
Eigen::Matrix<T, 3, 3> pitchMatrix(Angle<T> p) {
  Eigen::Matrix<T, 3, 3> P;
  P << 1, 0, 0,
      0, cos(p), -sin(p),
      0, sin(p), cos(p);
  return P;
}

template <typename T>
Eigen::Matrix<T, 3, 3> rollMatrix(Angle<T> r) {
  Eigen::Matrix<T, 3, 3> R;
  R << cos(r), 0, sin(r),
       0, 1, 0,
       -sin(r), 0, cos(r);
  return R;
}

/*
 * Implementation of the rotation, as described
 * previously. I think this representation makes a
 * lot of sense for performing optimization.
 * I am not sure this function should go
 * here, but I will keep it here, because maybe the
 * mapping of AbsoluteOrientation to a rotation matrix
 * is different in some other part of the code.
 */
template <typename T>
Eigen::Matrix<T, 3, 3> orientationToMatrix(
    const AbsoluteBoatOrientation<T> &orient) {
  return headingMatrix(orient.heading)
      *pitchMatrix(orient.pitch)
      *rollMatrix(orient.roll);
}


}  // namespace sail

#endif  // NAUTICAL_ABSOLUTE_ORIENTATION_H
