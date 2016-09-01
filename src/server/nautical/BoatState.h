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
#include <server/common/logging.h>
#include <server/nautical/AbsoluteOrientation.h>

/*

Attached to the sea surface locally at the boat,
there is a coordinate system, with the X axis
pointing towards the east and the Y axis pointing
towards the north, and Z pointing up. The boat is assumed
to have a unit rotation matrix, that is no rotation. And
the heading of the boat is 0 degrees. Increasing the heading
will rotate the boat clockwise in this plane.


                Y (north)
                ^
                |
                |
                |
                |
                #
               ###
               ###
              #####
              #####
             #######
             #######
             #######
            #########
            #########
            #########
            #########
            ### oZ ##--------------------->X (east)
            #########
            #########
            #########
            #########
            #########
             #######
             #######
             #######
             #######
              #####

  We want to make sure that we get the mapping of our
  AbsoluteOrientation object right.

  AbsoluateOrienation has (heading, roll, pitch).

  That object defines a
  rotation, that when applied to a vector in the boat coordinate
  system, produces a vector in the local coordinate system of
  the earth at the location of the boat.

  From the illustration above, it should be apparent that
  the vector in which the boat is heading is (0, 1, 0),
  a vector pointing towards starboard is (1, 0, 0) and the
  mast is pointing in the direction (0, 0, 1).

Lets look at the Y-axis (that is the heading) for different configurations.

For an orientation of (h, 0, 0), the Y axis should map to
(sin(h), cos(h), 0). If h=0, it maps to (0, 1, 0), that is the
same as for the boat.

If, in addition to heading h, there is some roll r, we
have an orienation (h, r, 0), but the mapping of the Y-axis
should be unaffected, that is we get
(sin(h), cos(h), 0)

But if there is some pitch p also, we have orientation
(h, r, p) and it should be

(sin(h)*cos(p), cos(h)*cos(p), sin(p))



What about the X-axis of the boat. If we only have heading h,
then (1, 0, 0) maps to

(cos(h), -sin(h), 0)

If there, in addition to heading, is some roll r, it should be
(cos(h)*cos(r), -sin(h)*cos(r), sin(r))

But the transformation of the X axis is unaffected by some pitch p.

What about the Z axis (which is the mast)?
That is pretty simple. We just compute the cross-product
of X with Y.

X = (sin(h)*cos(p), cos(h)*cos(p), sin(p))
Y = (cos(h)*cos(r), -sin(h)*cos(r), sin(r))

Z = cross(X, Y) = ...

Do we always get an orthonormal basis this way?

 */

namespace sail {

template <typename T>
Eigen::Matrix<T, 3, 3> orientationToMatrix(
    const TypedAbsoluteOrientation<T> &orient) {
  auto h = orient.heading;
  auto r = orient.roll;
  auto p = orient.pitch;
  Eigen::Matrix<T, 3, 3> R;

  // Based on the calculations above. Remember that
  // the columns of the transformation matrix are the
  // images of the base vectors, if the base vectors
  // are orthonormal.

  //   Image of X       Image of Y        Image of Z
  R << sin(h)*cos(p),   cos(h)*cos(r),    cos(h)*cos(p)*sin(r) - sin(p)*sin(h)*cos(r),
       cos(h)*cos(p),   -sin(h)*cos(r),   sin(p)*cos(h)*cos(r) - sin(h)*sin(r)*cos(p),
       sin(p),          sin(r),           -cos(p)*cos(r);
  return R;
}

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

  BoatState() {}
  BoatState(
      const GeographicPosition<T> &position,
      const HorizontalMotion<T> &bog,
      const HorizontalMotion<T> &windOverGround,
      const HorizontalMotion<T> &currentOverGround,
      const TypedAbsoluteOrientation<T> &orientation) :
        _position(position),
        _boatOverGround(bog),
        _windOverGround(windOverGround),
        _currentOverGround(currentOverGround),
        _orientation(orientation) {}


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
        && isFinite(_orientation);
  }

  Eigen::Matrix<T, 3, 1> worldHeadingVector() const {
    CHECK(false); //TODO
    return Eigen::Matrix<T, 3, 1>::Zero();
  }

  Eigen::Matrix<T, 2, 1> worldHeadingVectorHorizontal() const {
    CHECK(false); //TODO
    return Eigen::Matrix<T, 3, 1>::Zero();
  }

  Angle<T> heading() const {
    return _orientation.heading;
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

  const TypedAbsoluteOrientation<T> &orientation() const {
    return _orientation;
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

  TypedAbsoluteOrientation<T> _orientation;
};



template <typename T>
BoatState<T> interpolate(T lambda,
                         const BoatState<T> &a,
                         const BoatState<T> &b) {
  return BoatState<T>(
      interpolate<T>(lambda, a.position(), b.position()),
      interpolateAnything(lambda, a.boatOverGround(),
                                  b.boatOverGround()),
      interpolateAnything(lambda, a.boatOverGround(),
                                  b.boatOverGround()),
      interpolateAnything(lambda, a.boatOverGround(),
                                  b.boatOverGround()),
      interpolate(lambda, a.orientation(), b.orientation()));


}

} /* namespace sail */

#endif /* SERVER_NAUTICAL_BOATSTATE_H_ */
