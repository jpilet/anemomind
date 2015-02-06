/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/JsonIO.h>
#include <Poco/JSON/ParseHandler.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <fstream>

namespace sail {
namespace json {

Poco::Dynamic::Var load(const std::string &filename) {
  std::ifstream file(filename);
  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());
  parser.setHandler(handler);
  try {
    parser.parse(file);
    return handler->asVar();
  } catch (Poco::Exception &e) {
    return Poco::Dynamic::Var();
  }
}

void save(const std::string &filename, Poco::Dynamic::Var x) {
  std::ofstream file(filename);
  Poco::JSON::Stringifier::stringify(x, file, 0, 0);
}


}
}



