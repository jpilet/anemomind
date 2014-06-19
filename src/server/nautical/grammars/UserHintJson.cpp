/*
 *  Created on: 2014-06-19
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "UserHintJson.h"
#include <server/nautical/grammars/UserHint.h>
#include <Poco/JSON/Object.h>
#include <server/common/JsonPrimitive.h>
#include <server/common/TimeStampJson.h>
#include <server/common/JsonObjDeserializer.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const UserHint &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("type", x.typeAsString());
  obj->set("time", serialize(x.time()));
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, UserHint *dst) {
  ObjDeserializer deser(src);
  std::string type;
  TimeStamp t;
  deser.get("type", &type);
  deser.get("time", &t);
  if (deser.success()) {
    *dst = UserHint(type, t);
    return true;
  }
  return false;
}


}
} /* namespace sail */
