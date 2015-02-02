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

TEST(NavalSimulationJsonTest, SerializeDeserialize) {
  using namespace sail;
  NavalSimulation sim = makeNavSimConstantFlow();
  std::stringstream file;
  {
    auto simj = json::serialize(sim);
    std::cout << "Serialized. Now put it into a file" << std::endl;
    Poco::JSON::Stringifier::stringify(simj, file, 0, 0);
    std::cout << "Put into a file" << std::endl;
  }

  {
    NavalSimulation sim2;
    EXPECT_TRUE(json::deserialize(readVar(file), &sim2));
  }
}
