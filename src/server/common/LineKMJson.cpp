/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LineKMJson.h"
#include <server/common/LineKM.h>
#include <server/common/Json.h>
#include <server/common/JsonPrimitive.h>

#include <server/common/Json.impl.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const LineKM &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("k", serialize(x.getK()));
  obj->set("m", serialize(x.getM()));
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, LineKM *dst) {
  try {
    double k = 0, m = 0;
    Poco::JSON::Object::Ptr obj = src.extract<Poco::JSON::Object::Ptr>();
    if (!deserialize(obj->get("k"), &k) || !deserialize(obj->get("m"), &m)) {
      return false;
    }
    *dst = LineKM(k, m);
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}



}
} /* namespace sail */
