/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "SynthBoatTrajectory.h"
#include <cmath>
#include <server/common/string.h>
#include <iostream>
#include <server/common/LineKM.h>

namespace sail {


void SynthBoatTrajectory::WayPt::solveCircleTangentLine(
      const WayPt &a, bool posa,
      const WayPt &b, bool posb,
      Angle<double> *outAngleA, Angle<double> *outAngleB) {
  Vectorize<Length<double>, 2> dif = a.pos - b.pos;
  double C = sqrt(sqr(dif[0].meters()) + sqr(dif[1].meters()));
  double r = (posa? 1 : -1)*a.radius.meters();
  double t = (posb? 1 : -1)*b.radius.meters();
  double u = atan2(dif[0].meters(), dif[1].meters());
  double sol1 = asin((t - r)/C);
  double sol2 = M_PI - sol1;
  *outAngleA = Angle<double>::radians(sol1 - u);
  *outAngleB = Angle<double>::radians(sol2 - u);
}

namespace {
  Angle<double> makeCCW(Angle<double> x, bool positive) {
    return x + Angle<double>::degrees(positive? 0 : 180);
  }
}

SynthBoatTrajectory::WayPt::Connection
  SynthBoatTrajectory::WayPt::makeConnection(const WayPt &a, bool posa,
                                             const WayPt &b, bool posb) {
  Angle<double> sol1, sol2;
  solveCircleTangentLine(a, posa, b, posb,
      &sol1, &sol2);

  SynthBoatTrajectory::WayPt::Connection cona(a, b, makeCCW(sol1, posa),
      makeCCW(sol1, posb));
  SynthBoatTrajectory::WayPt::Connection conb(a, b, makeCCW(sol2, posa),
      makeCCW(sol2, posb));

  if (cona.isValid(a, posa)) {
    return cona;
  }
  assert(conb.isValid(a, posb));
  return conb;
}

bool SynthBoatTrajectory::WayPt::roundPositive(
    const WayPt &pred, const WayPt &succ) const {
  auto a = pos - pred.pos;
  auto b = succ.pos - pos;
  auto aNormal = Vectorize<Length<double>, 2>{-a[1], a[0]};
  return b[0].meters()*aNormal[0].meters() + b[1].meters()*aNormal[1].meters() > 0;
}

namespace {
  Arrayb calcRoundingDirections(Array<SynthBoatTrajectory::WayPt> waypoints) {
    assert(waypoints.size() >= 3);
    // The rounding direction for each buoy
    int count = waypoints.size();
    Arrayb positive = Arrayb::fill(count, false);
    for (int i = 1; i < count-1; i++) {
      positive[i] = waypoints[i].roundPositive(waypoints[i-1], waypoints[i+1]);
    }

    // Let the endpoint buoys inherit the rounding
    // directions of their neighbours.
    positive[0] = positive[1];
    positive[count-1] = positive[count-2];
    return positive;
  }

  Array<SynthBoatTrajectory::WayPt::Connection> makeConnections(
      Array<SynthBoatTrajectory::WayPt> waypts,
      Arrayb positive) {
      int count = waypts.size() - 1;
      Array<SynthBoatTrajectory::WayPt::Connection> cons(count);
      for (int i = 0; i < count; i++) {
        cons[i] = SynthBoatTrajectory::WayPt::makeConnection(waypts[i], positive[i],
                                                             waypts[i+1], positive[i+1]);
      }
      return cons;
  }
}

Array<SynthBoatTrajectory::CircleSegment>
SynthBoatTrajectory::makeCircleSegments(
          Array<SynthBoatTrajectory::WayPt> waypts,
          Array<SynthBoatTrajectory::WayPt::Connection> cons,
          Arrayb positive) {
    int count = cons.size() - 1;
  Array<SynthBoatTrajectory::CircleSegment> segments(count);
  Angle<double> oneRound = Angle<double>::degrees(360);
  Angle<double> zero = Angle<double>::degrees(0);
  for (int i = 0; i < count; i++) {
    Angle<double> fromAngle = cons[i + 0].dstAngle;
    Angle<double> rawToAngle   = cons[i + 1].srcAngle;
    int ptIndex = i+1;
    bool pos = positive[ptIndex];
    Angle<double> high = (pos? oneRound : zero);
    Angle<double> low  = (pos? zero : -oneRound);
    Angle<double> dif = (rawToAngle - fromAngle).moveToInterval(low, high);
    Angle<double> toAngle = fromAngle + dif;
    double frac = std::abs(dif.degrees()/360.0);
    WayPt pt = waypts[ptIndex];
    segments[i] = CircleSegment(pt,
        frac*(pt.circumference()),
        fromAngle, toAngle);
  }
  return segments;
}

const SynthBoatTrajectory::Segment &SynthBoatTrajectory::getSegmentByIndex(int index) const {
  int mainIndex = index/2;
  if (index - 2*mainIndex == 0) {
    return _lineSegments[mainIndex];
  } else {
    return _circleSegments[mainIndex];
  }
}

SynthBoatTrajectory::SynthBoatTrajectory(Array<WayPt> waypoints) {
  Arrayb positive = calcRoundingDirections(waypoints);
  Array<SynthBoatTrajectory::WayPt::Connection> connections =
      makeConnections(waypoints, positive);
  _lineSegments = connections.map<LineSegment>(
      [&](const SynthBoatTrajectory::WayPt::Connection &con) {
        return LineSegment(con.src, con.dst);
  });
  _circleSegments = makeCircleSegments(waypoints, connections, positive);

  int n = segmentCount();
  Arrayd props(n);
  for (int i = 0; i < n; i++) {
    props[i] = getSegmentByIndex(i).length().meters();
  }
  _indexer = ProportionateIndexer(props);
}

SynthBoatTrajectory::LineSegment::LineSegment(const ProjectedPosition &src,
    const ProjectedPosition &dst) {
  Vectorize<Length<double>, 2> dif = (dst - src);
  double len = sqrt(sqr(dif[0].meters()) + sqr(dif[1].meters()));
  _length = Length<double>::meters(len);
  _xMapMeters = LineKM(0, len, src[0].meters(), dst[0].meters());
  _yMapMeters = LineKM(0, len, src[1].meters(), dst[1].meters());
}

SynthBoatTrajectory::CurvePoint SynthBoatTrajectory::map(Length<double> at) const {
  auto r = _indexer.getBySum(at.meters());
  return getSegmentByIndex(r.index).map(Length<double>::meters(r.localX));
}





} /* namespace mmm */
