/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/SynthBoatTrajectory.h>
#include <armadillo>
#include <server/common/string.h>

using namespace sail;

namespace {
  SynthBoatTrajectory::WayPt makeWayPt(double x, double y, double r) {
    return SynthBoatTrajectory::WayPt(Vectorize<Length<double>, 2>{Length<double>::meters(x),
      Length<double>::meters(y)}, Length<double>::meters(r));
  }
}

TEST(SynthBoatTrajectoryTest, TangentTest1) {
  auto a = makeWayPt(0.5, 0.5, 0.5);
  auto b = makeWayPt(1.5, 1.5, 0.5);

  Angle<double> sol1, sol2;
  SynthBoatTrajectory::WayPt::solveCircleTangentLine(a, true, b, true, &sol1, &sol2);
  sol1 = sol1.normalizedAt0();
  sol2 = sol2.normalizedAt0();
  double minsol = std::min(sol1.degrees(), sol2.degrees());
  double maxsol = std::max(sol1.degrees(), sol2.degrees());


  EXPECT_NEAR(maxsol, 135.0, 1.0e-5);
  EXPECT_NEAR(minsol, -45, 1.0e-5);
}

TEST(SynthBoatTrajectoryTest, TangentTest2) {
  auto a = makeWayPt(0.5, 0.5, 0.5);
  auto b = makeWayPt(1.5, 1.5, 0.5);

  Angle<double> sol1, sol2;
  SynthBoatTrajectory::WayPt::solveCircleTangentLine(a, true, b, false, &sol1, &sol2);
  sol1 = sol1.positiveMinAngle();
  sol2 = sol2.positiveMinAngle();
  double minsol = std::min(sol1.degrees(), sol2.degrees());
  double maxsol = std::max(sol1.degrees(), sol2.degrees());

  // 90 0
  EXPECT_NEAR(maxsol, 90, 1.0e-5);
  EXPECT_NEAR(minsol, 0, 1.0e-5);
}

TEST(SynthBoatTrajectoryTest, TangentTest3) {
  auto a = makeWayPt(0.5, 0.5, 0.5);
  auto b = makeWayPt(1.5, 1.5, 0.5);

  Angle<double> sol1, sol2;
  SynthBoatTrajectory::WayPt::solveCircleTangentLine(a, false, b, true, &sol1, &sol2);
  sol1 = sol1.positiveMinAngle();
  sol2 = sol2.positiveMinAngle();
  double minsol = std::min(sol1.degrees(), sol2.degrees());
  double maxsol = std::max(sol1.degrees(), sol2.degrees());


  EXPECT_NEAR(maxsol, 270, 1.0e-5);
  EXPECT_NEAR(minsol, 180, 1.0e-5);
}

namespace {

  SynthBoatTrajectory::WayPt fitWayPt(Angle<double> a, Angle<double> b, double rad) {
    arma::mat A(2, 2);
    A(0, 0) = cos(a);
    A(0, 1) = sin(a);
    A(1, 0) = cos(b);
    A(1, 1) = sin(b);
    arma::mat B(2, 1);
    B(0, 0) = rad;
    B(1, 0) = rad;
    arma::mat X = arma::solve(A, B);
    return makeWayPt(X[0], X[1], rad);
  }
}

TEST(SynthBoatTrajectoryTest, TangentTest4) {
  Angle<double> angleA = Angle<double>::degrees(90);
  Angle<double> angleB = Angle<double>::degrees(270 + 45);

  auto v = fitWayPt(Angle<double>::degrees(90),
      Angle<double>::degrees(0), 1.0);
  EXPECT_NEAR(v.pos[0].meters(), 1.0, 1.0e-6);
  EXPECT_NEAR(v.pos[1].meters(), 1.0, 1.0e-6);
  EXPECT_NEAR(v.radius.meters(), 1.0, 1.0e-6);

  auto a = fitWayPt(angleA, angleB, 3.4);
  auto b = fitWayPt(angleA, angleB, 9.76);

  Angle<double> sol1, sol2;
  SynthBoatTrajectory::WayPt::solveCircleTangentLine(a, true, b, true, &sol1, &sol2);
  sol1 = sol1.positiveMinAngle();
  sol2 = sol2.positiveMinAngle();
  double minsol = std::min(sol1.degrees(), sol2.degrees());
  double maxsol = std::max(sol1.degrees(), sol2.degrees());
  EXPECT_NEAR(minsol, angleB.degrees() - 180.0, 1.0e-5);
  EXPECT_NEAR(maxsol, angleA.degrees() + 180.0, 1.0e-5);
}

TEST(SynthBoatTrajectoryTest, ConnectionTest) {
  auto a = makeWayPt(0, 0, 1);
  auto b = makeWayPt(3, 0, 1);

  {
    SynthBoatTrajectory::WayPt::Connection cona = SynthBoatTrajectory::WayPt::makeConnection(a, false, b, false);
    EXPECT_NEAR(cona.srcAngle.positiveMinAngle().degrees(), 90, 1.0e-6);
    EXPECT_NEAR(cona.dstAngle.positiveMinAngle().degrees(), 90, 1.0e-6);
    EXPECT_NEAR(cona.src[0].meters(), 0.0, 1.0e-6);
    EXPECT_NEAR(cona.src[1].meters(), 1.0, 1.0e-6);
    EXPECT_NEAR(cona.dst[0].meters(), 3.0, 1.0e-6);
    EXPECT_NEAR(cona.dst[1].meters(), 1.0, 1.0e-6);
  }{
    SynthBoatTrajectory::WayPt::Connection cona = SynthBoatTrajectory::WayPt::makeConnection(a, true, b, true);
    EXPECT_NEAR(cona.srcAngle.positiveMinAngle().degrees(), 270, 1.0e-6);
    EXPECT_NEAR(cona.dstAngle.positiveMinAngle().degrees(), 270, 1.0e-6);
    EXPECT_NEAR(cona.src[0].meters(), 0.0, 1.0e-6);
    EXPECT_NEAR(cona.src[1].meters(), -1.0, 1.0e-6);
    EXPECT_NEAR(cona.dst[0].meters(), 3.0, 1.0e-6);
    EXPECT_NEAR(cona.dst[1].meters(), -1.0, 1.0e-6);
  }{
    SynthBoatTrajectory::WayPt::Connection cona = SynthBoatTrajectory::WayPt::makeConnection(b, false, a, false);
    EXPECT_NEAR(cona.srcAngle.positiveMinAngle().degrees(), 270, 1.0e-6);
    EXPECT_NEAR(cona.dstAngle.positiveMinAngle().degrees(), 270, 1.0e-6);
    EXPECT_NEAR(cona.src[0].meters(), 3.0, 1.0e-6);
    EXPECT_NEAR(cona.src[1].meters(), -1.0, 1.0e-6);
    EXPECT_NEAR(cona.dst[0].meters(), 0.0, 1.0e-6);
    EXPECT_NEAR(cona.dst[1].meters(), -1.0, 1.0e-6);
  }
}

TEST(SynthBoatTrajectoryTest, RoundingDirectionTest) {
  auto pred = makeWayPt(0, 0, 1);
  auto aPos = makeWayPt(1, -0.5, 1);
  auto aNeg = makeWayPt(1, +0.5, 1);
  auto succ = makeWayPt(2, 0, 1);
  EXPECT_TRUE(aPos.roundPositive(pred, succ));
  EXPECT_FALSE(aNeg.roundPositive(pred, succ));
}

TEST(SynthBoatTrajectoryTest, LengthTest2) {
  double r = 0.5;
  Array<SynthBoatTrajectory::WayPt> pts(3);
  double circumference = 2.0*M_PI*r;
  pts[0] = makeWayPt(0, 0, r);
  pts[1] = makeWayPt(0, 2, r);
  pts[2] = makeWayPt(3, 2, r);
  SynthBoatTrajectory traj(pts);
  EXPECT_NEAR(traj.length().meters(), 2.0 + 0.25*circumference + 3.0, 1.0e-4);
}



