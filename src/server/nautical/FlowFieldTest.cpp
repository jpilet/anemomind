/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/FlowField.h>
#include <gtest/gtest.h>

using namespace sail;


namespace {

  FlowField makeTestFlowField() {
    Span<Length<double> > spSpan(Length<double>::nauticalMiles(-2),
                                   Length<double>::nauticalMiles(2));
    Span<Duration<double> > timeSpan(Duration<double>::hours(-1), Duration<double>::hours(1));
    Length<double> spRes = Length<double>::nauticalMiles(0.5);
    Duration<double> timeRes = Duration<double>::hours(0.5);

    FlowField::FlowVector meanFlow{Velocity<double>::knots(0.47), Velocity<double>::knots(0.4)};
    Velocity<double> maxDif = Velocity<double>::knots(0.0);

    return FlowField::generate(spSpan, spSpan, timeSpan,
                                spRes, timeRes,
                                meanFlow,
                                maxDif,
                                3, 3);
  }

}

TEST(FlowFieldTest, InstantiationAndMapping) {
  FlowField ff = makeTestFlowField();

  Length<double> x = Length<double>::nauticalMiles(0.0);
  Length<double> y = Length<double>::nauticalMiles(0.0);
  Duration<double> t = Duration<double>::hours(0.0);
  FlowField::FlowVector fv = ff.map(x, y, t);
  const double marg = 1.0e-6;
  EXPECT_NEAR(fv[0].knots(), 0.47, marg);
  EXPECT_NEAR(fv[1].knots(), 0.4, marg);
}


