/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/FlowField.h>
#include <server/common/Uniform.h>

using namespace sail;

namespace {

  FlowField::FlowVector makeMeanFlow() {
    return FlowField::FlowVector{Velocity<double>::knots(1.2), Velocity<double>::knots(0.29)};
  }
  FlowField generateBasicFlowField() {
    return FlowField::generate(Span<Length<double> >(Length<double>::nauticalMiles(-2),
                                                     Length<double>::nauticalMiles(2)),
                               Span<Length<double> >(Length<double>::nauticalMiles(1),
                                                     Length<double>::nauticalMiles(3)),

                              Span<Duration<double> >(Duration<double>::hours(0), Duration<double>::hours(1.5)),
                              Length<double>::meters(500),
                              Duration<double>::minutes(15.0),
                              makeMeanFlow(),
                              Velocity<double>::knots(0.0),
                              3, 3);
    }
  }


TEST(FlowField, Generate) {
  Uniform::initialize(1400755291);
  FlowField ff = generateBasicFlowField();
  Length<double> x = Length<double>::nauticalMiles(0.3);
  Length<double> y = Length<double>::nauticalMiles(1.4987);
  Duration<double> t = Duration<double>::minutes(34.324);
  FlowField::FlowVector fvec = ff.map(x, y, t);

  FlowField::FlowVector ref = makeMeanFlow();
  EXPECT_NEAR(fvec[0].metersPerSecond(), ref[0].metersPerSecond(), 1.0e-6);
  EXPECT_NEAR(fvec[1].metersPerSecond(), ref[1].metersPerSecond(), 1.0e-6);
}
