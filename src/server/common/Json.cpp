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

void deserializeField(CommonJson::Ptr cobj, const std::string &fieldName, std::string *valueOut) {
  Poco::JSON::Object::Ptr &obj = cobj->toObject()->get();
  if (obj->has(fieldName)) {
    *valueOut = obj->getValue<std::string>(fieldName);
  } else {
    *valueOut = "";
  }
}


}
}


