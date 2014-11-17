/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SYNTHBOATTRAJECTORY_H_
#define SYNTHBOATTRAJECTORY_H_

#include <server/nautical/GeographicReference.h>
#include <server/common/Array.h>

namespace sail {


class SynthBoatTrajectory {
 public:
  typedef GeographicReference::ProjectedPosition ProjectedPosition;

  /*
   * This WayPt is essentially a circle, with a radius.
   * When we round the waypoint, we follow part of its
   * circular arc.
   */
  class WayPt {
   public:
    WayPt(const ProjectedPosition &p, Length<double> r) :
      pos(p), radius(r) {}

    const Length<double> radius;
    const ProjectedPosition pos;

    /*
     * Used to compute the angle of the normal vector of a
     * line that is tangent to two waypoint circles.
     */
    static void solveCircleTangentLine(const WayPt &a, bool posa,
                                       const WayPt &b, bool posb,
                                   Angle<double> *outAngleA, Angle<double> *outAngleB);

  };

  SynthBoatTrajectory(Array<WayPt> waypoints);

  Length<double> length() const;
  ProjectedPosition map(Length<double> at) const;
 private:
};

} /* namespace mmm */

#endif /* SYNTHBOATTRAJECTORY_H_ */
