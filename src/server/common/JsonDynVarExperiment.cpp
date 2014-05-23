/*
 *  Created on: May 23, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <server/common/Array.h>
#include <server/common/logging.h>

using namespace sail;

/*
 * Main problem:
 *
 * Converting a Poco::Dynamic::Var to a Json array or object requires implementing
 * the VarHolder class, which seems a bit messy.
 */

namespace {
  Poco::Dynamic::Var serialize(int x) {
    return Poco::Dynamic::Var(x);
  }

  template <typename T>
  Poco::Dynamic::Var serialize(sail::Array<T> srcarr) {
    Poco::JSON::Array dstarr;

    for (auto e : srcarr) {
      dstarr.add(serialize(e));
    }

    return Poco::Dynamic::Var(dstarr);
  }

  void stringify(Poco::Dynamic::Var x,
      std::stringstream &ss, int a, int b) {
    try {
      x.convert<Poco::JSON::Object::Ptr>()->stringify(ss, a, b); // <-- DOESN'T COMPILE BECAUSE NO VarHolder FOR THIS TYPE
    } catch (...) {}

    try {
      x.convert<Poco::JSON::Array::Ptr>()->stringify(ss, a, b); // <-- DOESN'T COMPILE BECAUSE NO VarHolder FOR THIS TYPE
    } catch (...) {}

    LOG(FATAL) << "FAILED TO STRINGIFY";

  }
}


int main() {
  {
    Arrayi data(3);
    data.setTo(119);
    Poco::Dynamic::Var test = serialize(data);
  }

  return 0;
}


