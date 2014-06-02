/*
 *  Created on: 2014-04-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Json.h"
#include <server/common/logging.h>

namespace sail {
namespace json {


void serializeField(Poco::JSON::Object::Ptr obj, const std::string &fieldName, const std::string &value) {
  if (!value.empty()) {
    obj->set(fieldName, value);
  }
}

void deserializeField(Poco::Dynamic::Var cobj, const std::string &fieldName, std::string *valueOut) {
  Poco::JSON::Object::Ptr obj = cobj.extract<Poco::JSON::Object::Ptr>();
  if (obj->has(fieldName)) {
    *valueOut = obj->getValue<std::string>(fieldName);
  } else {
    *valueOut = "";
  }
}

void stringify(Poco::Dynamic::Var x, std::ostream *out, unsigned int indent, int step) {
  if (x.type() == typeid(Poco::JSON::Object::Ptr)) {
    x.extract<Poco::JSON::Object::Ptr>()->stringify(*out, indent, step);
  } else if (x.type() == typeid(Poco::JSON::Array::Ptr)) {
    x.extract<Poco::JSON::Array::Ptr>()->stringify(*out, indent, step);
  } else {
    LOG(FATAL) << "Unable to stringify object";
  }
}


}
}


