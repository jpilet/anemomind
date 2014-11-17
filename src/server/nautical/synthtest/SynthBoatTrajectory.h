/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SYNTHBOATTRAJECTORY_H_
#define SYNTHBOATTRAJECTORY_H_

#include <server/nautical/GeographicReference.h>
#include <server/common/Array.h>
#include <server/common/LineKM.h>

namespace sail {


class SynthBoatTrajectory {
 public:
  typedef GeographicReference::ProjectedPosition ProjectedPosition;

  /*
   * A WayPt is a buoy that has a positition 'pos' that
   * we round with a radius 'radius'.
   *
   * The rounding direction is determined by its predecessor
   * and successor.
   */
  class WayPt {
   public:
    WayPt() {}
    WayPt(const ProjectedPosition &p, Length<double> r) :
      pos(p), radius(r) {}

    Length<double> radius;
    ProjectedPosition pos;

    Length<double> circumference() const {
      return 2.0*M_PI*radius;
    }

    ProjectedPosition evalPos(Angle<double> theta) const {
      return ProjectedPosition{pos[0] + radius*cos(theta),
                               pos[1] + radius*sin(theta)};
    }

    // Normalized derivative of the above function, w.r.t. theta.
    Vectorize<double, 2> tangent(Angle<double> theta, bool positive) const {
      int sign = (positive? 1 : -1);
      return Vectorize<double, 2>{-sign*sin(theta), sign*cos(theta)};
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


    /*
     * A class that specifies the connecting straight line
     * segment between to buoys to round. In addition it specifies
     * the angle on each buoy where the line segment touches it.
     *
     * Used only for internal purposes, but exposed publicly so that
     * it can be tested.
     */
    class Connection {
     public:
      Connection() {}
      Connection(const WayPt &s, const WayPt &d, Angle<double> sa,
        Angle<double> da) :
          src(s.evalPos(sa)), dst(d.evalPos(da)),
          srcAngle(sa), dstAngle(da) {}

      ProjectedPosition src, dst;
      Angle<double> srcAngle, dstAngle;

      // Returns true if the trajectory from 'src' to 'dst' is coherent
      // with the orientation of 'srcPt'. The orientation is given by
      // 'positive', with positive=1 means counter-clockwise and
      // positive=0 means clockwise.
      bool isValid(const WayPt &srcPt, bool positive) const {
        Vectorize<double, 2> tgt = srcPt.tangent(srcAngle, positive);
        return (tgt[0]*(dst[0] - src[0]).meters() + tgt[1]*(dst[1] - src[1]).meters() > 0);
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


  class CurvePoint {
   public:
    CurvePoint(const ProjectedPosition &p, const Vectorize<double, 2> &t) :
      position(p), tangent(t) {}
     const ProjectedPosition position;
     const Vectorize<double, 2> tangent;
  };

  CurvePoint map(Length<double> at) const;

 private:
  class LineSegment {
   public:
    LineSegment() {}
    LineSegment(const ProjectedPosition &src,
        const ProjectedPosition &dst);
    CurvePoint map(Length<double> at) const {
      return CurvePoint();
    }
    Length<double> length() const {return _length;}
   private:
    LineKM _xMapMeters, _yMapMeters;
    Length<double> _length;
  };

  class CircleSegment {
   public:
    CircleSegment() {}

    CircleSegment(const WayPt &pt, Length<double> length,
        Angle<double> fromAngle, Angle<double> toAngle) :
          _pt(pt), _length(length),
          _lengthToAngle(0, length.meters(), fromAngle.radians(), toAngle.radians()) {}


    CurvePoint map(Length<double> at) const {
      Angle<double> theta = Angle<double>::radians(_lengthToAngle(at.meters()));
      return CurvePoint(_pt.evalPos(theta), _pt.tangent(theta, _lengthToAngle.getK() > 0));
    }

    Length<double> length() const {
      return _length;
    }
   private:
    WayPt _pt;
    Length<double> _length;
    LineKM _lengthToAngle;
  };

  static Array<SynthBoatTrajectory::CircleSegment>
      makeCircleSegments(
            Array<SynthBoatTrajectory::WayPt> waypts,
            Array<SynthBoatTrajectory::WayPt::Connection> cons,
            Arrayb positive);


  Array<LineSegment> _lineSegments;
  Array<CircleSegment> _circleSegments;
};

} /* namespace mmm */

#endif /* SYNTHBOATTRAJECTORY_H_ */
