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

    ProjectedPosition evalPos(Angle<double> theta) const {
      return ProjectedPosition{pos[0] + radius*cos(theta),
                               pos[1] + radius*sin(theta)};
    }

    // Normalized derivative of the above function, w.r.t. theta.
    Vectorize<double, 2> tangent(Angle<double> theta) const {
      return Vectorize<double, 2>{-sin(theta), cos(theta)};
    }

    /*
     * Used to compute the angle of the normal vector of a
     * line that is tangent to two waypoint circles.
     *
     * The 'posa' and 'posb' boolean arguments tell for
     * each way pt in which direction it should be rounded.
     */
    static void solveCircleTangentLine(const WayPt &a, bool posa,
                                       const WayPt &b, bool posb,
                                   Angle<double> *outAngleA, Angle<double> *outAngleB);

    class Connection {
     public:
      Connection(const WayPt &s, const WayPt &d, Angle<double> sa,
        Angle<double> da) :
          src(s.evalPos(sa)), dst(d.evalPos(da)),
          srcAngle(sa), dstAngle(da) {}

      const ProjectedPosition src, dst;
      const Angle<double> srcAngle, dstAngle;

      // Returns true if the trajectory from 'src' to 'dst' is coherent
      // with the orientation of 'srcPt'. The orientation is given by
      // 'positive', with positive=1 means counter-clockwise and
      // positive=0 means clockwise.
      bool isValid(const WayPt &srcPt, bool positive) const {
        Vectorize<double, 2> tgt = srcPt.tangent(srcAngle);
        return (tgt[0]*(dst[0] - src[0]).meters() + tgt[1]*(dst[1] - src[1]).meters() > 0) == positive;
      }
    };

    static Connection makeConnection(const WayPt &a, bool posa,
                              const WayPt &b, bool posb);


    /*
     * Based on preceding and succeeding way points,
     * should this way point be rounded in positive (counter-clockwise) direction?
     */
    bool roundPositive(const WayPt &pred, const WayPt &succ) const;
  };

  SynthBoatTrajectory(Array<WayPt> waypoints);

  Length<double> length() const;
  ProjectedPosition map(Length<double> at) const;
 private:
};

} /* namespace mmm */

#endif /* SYNTHBOATTRAJECTORY_H_ */
