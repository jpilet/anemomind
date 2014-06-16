/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/PhysicalQuantityJson.h>
#include <server/nautical/FlowField.h>
#include <server/math/GridJson.h>
#include <server/nautical/FlowFieldJson.h>
#include <server/common/LineKMJson.h>
#include <server/common/Json.h>
#include <server/common/string.h>
#include <gtest/gtest.h>
#include <Poco/JSON/Stringifier.h>

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

TEST(FlowFieldTest, Mapping) {
  FlowField ff = makeTestFlowField();

  Length<double> x = Length<double>::nauticalMiles(0.0);
  Length<double> y = Length<double>::nauticalMiles(0.0);
  Duration<double> t = Duration<double>::hours(0.0);
  FlowField::FlowVector fv = ff.map(x, y, t);
  const double marg = 1.0e-6;
  EXPECT_NEAR(fv[0].knots(), 0.47, marg);
  EXPECT_NEAR(fv[1].knots(), 0.4, marg);
}

TEST(FlowFieldTest, Json) {
  FlowField ff = makeTestFlowField();
  FlowField ff2;

  Poco::Dynamic::Var jsonobj = json::serialize(ff);

  std::cout << "Stringify grid:" << std::endl;
  Poco::JSON::Stringifier::stringify(json::serialize(ff.grid()), std::cout);
  std::cout << "Stringify flowvec:" << std::endl;
  Poco::JSON::Stringifier::stringify(json::serialize(ff.flow()[0]), std::cout);
  std::cout << "Stringify flow:" << std::endl;
  Poco::JSON::Stringifier::stringify(json::serialize(ff.flow()), std::cout);
  std::cout << "Stringify full obj :" << std::endl;
  Poco::JSON::Stringifier::stringify(jsonobj, std::cout);
  std::cout << "done." << std::endl;

  json::deserialize(jsonobj, &ff2);
  EXPECT_EQ(ff.grid(), ff2.grid());
  EXPECT_EQ(ff.flow(), ff2.flow());
  EXPECT_EQ(ff, ff2);
}



