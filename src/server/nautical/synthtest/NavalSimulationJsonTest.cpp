/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/NavalSimulationJson.h>
#include <Poco/JSON/ParseHandler.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <sstream>

namespace {

Poco::Dynamic::Var readVar(std::stringstream &ss) {
  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());
  parser.setHandler(handler);
  try {
    parser.parse(ss);
    return handler->asVar();
  } catch (Poco::Exception &e) {
    return Poco::Dynamic::Var();
  }
}

}

using namespace sail::NavCompat;

TEST(NavalSimulationJsonTest, SerializeDeserialize) {
  using namespace sail;
  NavalSimulation sim = makeNavSimConstantFlow();
  std::stringstream file;
  {
    auto simj = json::serialize(sim);
    Poco::JSON::Stringifier::stringify(simj, file, 0, 0);
  }

  {
    NavalSimulation sim2;
    EXPECT_TRUE(json::deserialize(readVar(file), &sim2));

    // Compare a couple of values
    EXPECT_EQ(getNav(sim.boatData(0).navs(), 30),
              getNav(sim2.boatData(0).navs(), 30));
    EXPECT_EQ(getNav(sim.boatData(1).navs(), 39),
              getNav(sim2.boatData(1).navs(), 39));
    EXPECT_EQ(sim.boatData(0).specs().samplingPeriod(),
              sim2.boatData(0).specs().samplingPeriod());
    EXPECT_EQ(sim.boatData(0).states()[3].trueState().boatSpeedThroughWater,
              sim2.boatData(0).states()[3].trueState().boatSpeedThroughWater);
  }
}
