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

                #
               ###
               ###
              #####
              #####
             #######
             #######
            #########
            #########
           ###########
           ###########
          #############
          ####  oZ(up)#   ------------------>X (east)
          #############
          #############
          #############
          #############
          #############
          #############
          #############
           ###########
           ###########
            #########

  Together, the vectors form an orthonormal basis.

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

  Assuming that we have an absolute orientation of
    (heading, roll, pitch) = (h, r, p), we are going to
  express the full rotation as a product of elementary
  rotation. First we apply a roll to the boat by r,
  then a pitch the boat by p, and finally we apply the heading.

  R(h, r, p) = H(h)*P(p)*R(r)

  with H(h) = cos(h)  sin(h) 0
              -sin(h) cos(h) 0
                   0       0 1

       P(p) = 1      0       0
              0 cos(p) -sin(p)
              0 sin(p)  cos(p)

       R(r) = cos(r)  0   sin(r)
              0       1        0
              -sin(r) 0   cos(r)

  Explanation:
    In the matrix product above, read the application
    of rotations from right to left. So first we apply
    the roll. An increasing roll means that the boat leans
    towards starboard, such as when the wind is coming
    from port side, or the crew moving to the starboard
    side of the boat.

    Then after we have rolled the boat, we apply some pitch.
    An increasing pitch means that the boat is tilted as if
    the crew would move towards the aft, or the boat climbing
    up a big wave.

    Then we apply the heading. An increasing heading means that
    the boat turns towards starboard, as if we were to
    round a buoy clockwise.

  Invariants to test:
   * No matter the roll and the pitch, and for a given heading,
     the transformed Y axis projected on the XY plan should
     span the same linear subspace as another transformed Y
     axis for the same heading, but any other roll and pitch.
     In other words, the heading of the boat should more or
     less be unaffected by the heal angle and the pitch.

 */

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

  BoatState() {}
  BoatState(
      const GeographicPosition<T> &position,
      const HorizontalMotion<T> &bog = HorizontalMotion<T>(),
      const HorizontalMotion<T> &windOverGround = HorizontalMotion<T>(),
      const HorizontalMotion<T> &currentOverGround = HorizontalMotion<T>(),
      const AbsoluteBoatOrientation<T> &orientation =
          AbsoluteBoatOrientation<T>()) :
        _position(position),
        _boatOverGround(bog),
        _windOverGround(windOverGround),
        _currentOverGround(currentOverGround),
        _orientation(orientation) {}


  // Vectors defined as in the illustration above.
  static Eigen::Matrix<T, 3, 1> localMastVector() {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(0.0), T(1.0));
  }
  static Eigen::Matrix<T, 3, 1> localHeadingVector() {
    return Eigen::Matrix<T, 3, 1>(T(0.0), T(1.0), T(0.0));
  }
  static Eigen::Matrix<T, 3, 1> localStarboardVector() {
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

  void setBoatOverGround(const HorizontalMotion<T> &src) {
    _boatOverGround = src;
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


  Eigen::Matrix<T, 3, 3> rotationMatrix() const {
    return orientationToMatrix(_orientation);
  }

  Eigen::Matrix<T, 3, 1> worldHeadingVector() const {
    return rotationMatrix().block(0, 1, 3, 1);
  }

  Angle<T> heading() const {
    return _orientation.heading;
  }

  // To be avoided: Undefined (NaN) when boatOverWater has 0 norm.
  Angle<T> leewayAngle() const {
    return heading() - boatOverWater().angle();
  }

  Angle<T> AWA() const {
    return computeAWA(_boatOverGround, _windOverGround, heading());
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

  const AbsoluteBoatOrientation<T> &orientation() const {
    return _orientation;
  }

  bool operator==(const BoatState<T> &other) const {
    return _position == other._position
        && _boatOverGround == other._boatOverGround
        && _windOverGround == other._windOverGround
        && _currentOverGround == other._currentOverGround
        && _orientation == other._orientation;
  }

  template <typename Function>
  BoatState<typename FunctionReturnType<Function, T>::type>
    mapObjectValues(Function f) const {
    return BoatState<typename FunctionReturnType<Function, T>::type>(
        _position.template mapObjectValues<Function>(f),
        _boatOverGround.template mapObjectValues<Function>(f),
        _windOverGround.template mapObjectValues<Function>(f),
        _currentOverGround.template mapObjectValues<Function>(f),
        _orientation.template mapObjectValues<Function>(f));
  }
private:
  // If we need time to, I think it is best
  // to put it inside a TimedValue, that is a
  // TimedValue<BoatState<T>>. That way, it becomes
  // natural to treat this object as a regular
  // measurement if we need to.

  GeographicPosition<T> _position;
  HorizontalMotion<T> _boatOverGround;
  HorizontalMotion<T> _windOverGround;
  HorizontalMotion<T> _currentOverGround;
  AbsoluteBoatOrientation<T> _orientation;
};



template <typename T>
BoatState<T> interpolate(T lambda,
                         const BoatState<T> &a,
                         const BoatState<T> &b) {
  return BoatState<T>(
      interpolate<T>(lambda, a.position(), b.position()),
      interpolateAnything(lambda, a.boatOverGround(),
                                  b.boatOverGround()),
      interpolateAnything(lambda, a.windOverGround(),
                                  b.windOverGround()),
      interpolateAnything(lambda, a.currentOverGround(),
                                  b.currentOverGround()),
      interpolate(lambda, a.orientation(), b.orientation()));


}

} /* namespace sail */

#endif /* SERVER_NAUTICAL_BOATSTATE_H_ */
