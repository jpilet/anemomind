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



