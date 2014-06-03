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

bool deserializeField(Poco::Dynamic::Var cobj, const std::string &fieldName, std::string *valueOut) {
  Poco::JSON::Object::Ptr obj = cobj.extract<Poco::JSON::Object::Ptr>();
  if (obj->has(fieldName)) {
    try {
      *valueOut = obj->getValue<std::string>(fieldName);
      return true;
    } catch (Poco::Exception &e) {
      return false;
    }
  } else {
    *valueOut = "";
    return false;
  }
}


}
}


