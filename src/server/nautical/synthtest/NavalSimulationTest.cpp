/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Testcases to test the Testcase class :-)
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/NavalSimulation.h>
#include <server/common/Functional.h>

using namespace sail;



TEST(TestcaseTest, Corruptor) {
  typedef CorruptedBoatState::Corruptor<Velocity<double > > Corr;

  Velocity<double> x;

  Corr corr(2.0,
      Velocity<double>::knots(1.0), Velocity<double>::knots(0.0));
  std::default_random_engine e;
  EXPECT_NEAR(corr.corrupt(Velocity<double>::knots(3.0), e).knots(), 7.0, 1.0e-6);
}

TEST(TestcaseTest, BoatSimulationSpecs) {
  Array<BoatSimulationSpecs::TwaDirective> dirs(2);
  dirs[0] = BoatSimulationSpecs::TwaDirective::constant(Duration<double>::minutes(3.0), Angle<double>::degrees(119));
  dirs[1] = BoatSimulationSpecs::TwaDirective::constant(Duration<double>::minutes(1.0), Angle<double>::degrees(32));
  BoatSimulationSpecs specs(BoatCharacteristics(), dirs, CorruptedBoatState::CorruptorSet());
  EXPECT_NEAR(specs.duration().minutes(), 4.0, 1.0e-6);
  EXPECT_NEAR(specs.twa(Duration<double>::minutes(2.99)).degrees(), 119.0, 1.0e-6);
  EXPECT_NEAR(specs.twa(Duration<double>::minutes(3.01)).degrees(), 32.0, 1.0e-6);

  std::default_random_engine e;
  EXPECT_NEAR(specs.corruptors().awa.corrupt(Angle<double>::degrees(34), e).degrees(), 34.0, 1.0e-6);
  EXPECT_NEAR(specs.corruptors().magHdg.corrupt(Angle<double>::degrees(34), e).degrees(), 34.0, 1.0e-6);
  EXPECT_NEAR(specs.corruptors().gpsBearing.corrupt(Angle<double>::degrees(34), e).degrees(), 34.0, 1.0e-6);
  EXPECT_NEAR(specs.corruptors().aws.corrupt(Velocity<double>::knots(34), e).knots(), 34.0, 1.0e-6);
  EXPECT_NEAR(specs.corruptors().watSpeed.corrupt(Velocity<double>::knots(34), e).knots(), 34.0, 1.0e-6);
  EXPECT_NEAR(specs.corruptors().gpsSpeed.corrupt(Velocity<double>::knots(34), e).knots(), 34.0, 1.0e-6);
}

TEST(TestcaseTest, MakeTestcase) {
  HorizontalMotion<double> wind = HorizontalMotion<double>::polar(
      Velocity<double>::metersPerSecond(7.3),
      Angle<double>::degrees(129));

  HorizontalMotion<double> current = HorizontalMotion<double>::polar(
      Velocity<double>::knots(1.3),
      Angle<double>::degrees(77));


  TimeStamp timeoffset = TimeStamp::UTC(2012, 10, 13, 5, 34, 23);

  auto pos = GeographicPosition<double>(Angle<double>::degrees(30),
                                        Angle<double>::degrees(45));
  GeographicReference georef(pos);

  NavalSimulation::FlowFun windfun = NavalSimulation::constantFlowFun(wind);
  NavalSimulation::FlowFun currentfun = NavalSimulation::constantFlowFun(current);

  Array<BoatSimulationSpecs::TwaDirective> dirs(2);
  dirs[0] = BoatSimulationSpecs::TwaDirective::constant(Duration<double>::minutes(3.0), Angle<double>::degrees(119));
  dirs[1] = BoatSimulationSpecs::TwaDirective::constant(Duration<double>::minutes(1.0), Angle<double>::degrees(32));
  BoatSimulationSpecs specs(BoatCharacteristics(), dirs, CorruptedBoatState::CorruptorSet());

  std::default_random_engine e;
  NavalSimulation tc(e, georef, timeoffset, windfun, currentfun,
    Array<BoatSimulationSpecs>{specs});

  auto t = tc.toLocalTime(timeoffset);
  EXPECT_NEAR(t.seconds(), 0.0, 1.0e-6);
  auto m = tc.geoRef().map(pos);
  EXPECT_NEAR(m[0].meters(), 0.0, 1.0e-4);
  EXPECT_NEAR(m[1].meters(), 0.0, 1.0e-4);
  auto w0 = tc.wind()(m, t);
  EXPECT_NEAR(w0[0].metersPerSecond(), wind[0].metersPerSecond(), 1.0e-5);
  auto c0 = tc.current()(m, t);
  EXPECT_NEAR(c0[0].metersPerSecond(), current[0].metersPerSecond(), 1.0e-5);

  EXPECT_EQ(tc.boatCount(), 1);
  EXPECT_NEAR(tc.boatData(0).states().size(), 4*60, 2);
}

namespace {
  Array<HorizontalMotion<double> > getGroundTruth(
      NavalSimulation::BoatData boatData, bool wind) {
    return sail::map(
    boatData.states(),
    [=](const CorruptedBoatState &s) {
      return (wind?
              s.trueState().trueWind :
              s.trueState().trueCurrent);
    }).toArray();
  }
}

TEST(TestcaseTest, NoCorruption) {
  auto sim = makeNavSimConstantFlow();
  auto errors = sim.boatData(0).evaluateNoCalibration();
  EXPECT_LE(errors.wind().error().norm().mean().knots(), 1.0e-6);
  EXPECT_LE(errors.wind().error().norm().rms().knots(), 1.0e-6);
  EXPECT_LE(errors.current().error().norm().mean().knots(), 1.0e-6);
  EXPECT_LE(errors.current().error().norm().rms().knots(), 1.0e-6);

  {
    NavalSimulation::BoatData boatData = sim.boatData(1);
    Array<HorizontalMotion<double> > wind = getGroundTruth(boatData, true);
    Array<HorizontalMotion<double> > current = getGroundTruth(boatData, false);
    auto A = boatData.evaluateFitness(wind, Array<HorizontalMotion<double> >());
    auto B = boatData.evaluateFitness(Array<HorizontalMotion<double> >(), current);
    EXPECT_LE(A.wind().error().norm().mean().knots(), 1.0e-6); EXPECT_TRUE(A.current().error().norm().undefined());
    EXPECT_TRUE(B.wind().error().norm().undefined()); EXPECT_LE(B.current().error().norm().mean().knots(), 1.0e-6);
  }
}


