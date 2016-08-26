/*
 * BoatState.h
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 *
 * A BoatState represents that best knowledge that we have
 * about a boat at some point in time. All measurements that
 * it contains are corrected, and it does not contain any raw or
 * ambiguous data.
 *
 * A sequence of BoatStates is produced by sail::Reconstructor::reconstruct.
 * The BoatState can be safely consumed by the rest of the application,
 * without any worry that the data is wrong.
 */

#ifndef SERVER_NAUTICAL_BOATSTATE_H_
#define SERVER_NAUTICAL_BOATSTATE_H_

#include <Eigen/Dense>
#include <server/nautical/GeographicPosition.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/math/EigenUtils.h>
#include <server/nautical/common.h>

namespace sail {

template <typename T>
class BoatState;

template <typename T>
Angle<T> angleBlowingFrom(const HorizontalMotion<T> &m) {
  return (m.angle() + Angle<T>::radians(M_PI)).normalizedAt0();
}

template <typename T, int N>
struct UnitVector {
  static bool is(const Eigen::Matrix<T, N, 1> &v, T tol) {
    return std::abs(v.norm() - 1.0) < tol;
  }
};

// So that we can do all our error computations
// in the velocity domain. Angles are reinterpreted
// as velocity arc lengths.
template <typename T>
Velocity<T> arcLength(
    const Velocity<T> &refVelocity,
    const Angle<T> &angle) {
  return angle.radians()*refVelocity;
}


// The norm of the difference between two vectors of length
// 'refVelocity' and the angle 'angle' between them.
template <typename T>
Velocity<T> interpretAngleAsVelocity(
    const Velocity<T> &refVelocity,
    Angle<T> angle) {
  return (2.0*sin(0.5*angle))*refVelocity;
}

// The norm of the difference between 'motion', and another motion
// of the same magnitude but oriented by an angle 'angle'
// w.r.t. geographic north.
template <typename T>
Velocity<T> velocityDifferenceBetween(
    const HorizontalMotion<T> &motion,
    const Angle<T> angle) {
  return HorizontalMotion<T>(motion - HorizontalMotion<T>::polar(
      motion.norm(), angle)).norm();
}


template <typename T>
class BoatState {
public:
  static Eigen::Matrix<T, 3, 1> defaultAxis() {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(0.0), T(1.0));
  }

  BoatState() : _axis(defaultAxis()) {}
  BoatState(
      const GeographicPosition<T> &position,
      const HorizontalMotion<T> &bog,
      const HorizontalMotion<T> &windOverGround,
      const HorizontalMotion<T> &currentOverGround,
      Angle<T> angle,
      const Eigen::Matrix<T, 3, 1> &axis =
          defaultAxis()) :
        _position(position),
        _boatOverGround(bog),
        _windOverGround(windOverGround),
        _currentOverGround(currentOverGround),
        _angle(angle), _axis(axis) {}


  // This is how the orthonormal basis attached to the boat
  // relates to the boat. Any one of these vector can be expressed
  // as a cross product of the other two

  // When the boat is in a perfectly horizontal equilibrium, this
  // vector points up, pretty much along the mast.
  static Eigen::Matrix<T, 3, 1> mastVector() {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(0.0), T(1.0));
  }

  // This vector points in the direction of the heading of the boat.
  // When the boat is horizontal, this vector is parallel to the sea.
  static Eigen::Matrix<T, 3, 1> headingVector() {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(1.0), T(0.0));
  }

  // This vector points towards starboard, parallel to the sea surface.
  static Eigen::Matrix<T, 3, 1> starboardVector() {
    return Eigen::Matrix<T, 3, 1>(T(1.0), T(0.0), T(0.0));
  }

  const GeographicPosition<T> &position() const {return _position;}
  const HorizontalMotion<T> &boatOverGround() const {
    return _boatOverGround;
  }
  const HorizontalMotion<T> &windOverGround() const {
    return _windOverGround;
  }

  const HorizontalMotion<T> &currentOverGround() const {
    return _currentOverGround;
  }

  const Eigen::Matrix<T, 3, 1> &axis() const {
    return _axis;
  }

  const Eigen::Matrix<T, 3, 1> &angle() const {
    return _angle;
  }

  // Or should it be "over current"? But that doesn't sound quite right.
  HorizontalMotion<T> boatOverWater() const {
    return _boatOverGround - _currentOverGround;
  }

  HorizontalMotion<T> windOverWater() const {
    return _windOverGround - _currentOverGround;
  }

  HorizontalMotion<T> apparentWind() const {
    return _windOverGround - _boatOverGround;
  }

  bool valid() const {
    return isFinite(_position)
        && isFinite(_boatOverGround)
        && isFinite(_windOverGround)
        && isFinite(_currentOverGround)
        && isFinite(_angle)
        && isFinite(_axis)
        && UnitVector<T, 3>::is(_axis, T(1.0e-6));
  }

  Eigen::AngleAxis<T> angleAxis() const {
    return Eigen::AngleAxis<T>(_angle.radians(), _axis);
  }

  Eigen::Matrix<T, 3, 1> worldHeadingVector() const {
    return angleAxis()*headingVector();
  }

  Eigen::Matrix<T, 2, 1> worldHeadingVectorHorizontal() const {
    auto v = worldHeadingVector();
    return Eigen::Matrix<T, 2, 1>(v(0), v(1));
  }

  Angle<T> heading() const {
    auto v2 = worldHeadingVector();
    return headingAngle<T>(v2(0), v2(1));
  }

  // To be avoided: Undefined (NaN) when boatOverWater has 0 norm.
  Angle<T> leewayAngle() const {
    return heading() - boatOverWater().angle();
  }



  Angle<T> AWA() const {
    return angleBlowingFrom(apparentWind()) - heading();
  }





/////////////////////////////////////// Used to generate the residuals
////// Should the go elsewhere???

  // This is used to estimate drift.
    Velocity<T> computeLeewayError(
        // Leeway angle estimated using some drift model,
        // e.g. based on wind direction etc.
        const Angle<T> predictedLeewayAngle) const {
      auto bow = boatOverWater();
      return velocityDifferenceBetween<T>(bow, heading())
          - interpretAngleAsVelocity<T>(bow.norm(), predictedLeewayAngle);
    }

  Velocity<T> computeAWAError(const Angle<T> &measuredAWA) const {
    Angle<T> angleOfWindDirection = measuredAWA
        + Angle<T>::degrees(T(180.0)) + heading();
    return velocityDifferenceBetween(apparentWind(),
        angleOfWindDirection);
  }
private:
  GeographicPosition<T> _position;
  HorizontalMotion<T> _boatOverGround;
  HorizontalMotion<T> _windOverGround;
  HorizontalMotion<T> _currentOverGround;

  /*
   * A local, right-handed, coordinate system attached to the earth
   * at the current location of the boat:
   *  - X axis points from west to easy
   *  - Y axis points from south to north
   *  - Z axis points up, perpendicular to the earth. This
   *  vector is an axis-angle representation of a rotation, that when applied
   *  to a coordinate in the boat coordinate system results in a coordinate
   *  in this local earth-attached coordinate system.
   *
   */

  /* Special cases:
   *
   * If _axis = (0, 0, 1) then _angle == -heading()
   * If _axis = (0, 0, -1) then _angle == heading()
   *
   */
  Angle<T> _angle;

  // If we don't really know how to calibrate the inertial unit
  // we can just fix this axis to (0, 0, 1) and assume the boat just rotates
  // in the plane.
  Eigen::Matrix<T, 3, 1> _axis; // Should always have length 1.0
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_BOATSTATE_H_ */
