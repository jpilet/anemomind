/*
 *  Created on: 2014-04-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Json.h"
#include <server/common/logging.h>

namespace sail {
namespace json {

CommonJson::Ptr CommonJson::getObjectField(Poco::JSON::Object::Ptr src,
    std::string fieldName) {
    if (src->isArray(fieldName)) {
      return CommonJson::Ptr(new CommonJsonArray(src->getArray(fieldName)));
    } else if (src->isObject(fieldName)) {
      return CommonJson::Ptr(new CommonJsonObject(src->getObject(fieldName)));
    } else {
      return CommonJson::Ptr(new CommonJsonVar(src->get(fieldName)));
    }
}

CommonJson::Ptr CommonJson::getArrayElement(Poco::JSON::Array &src, int index) {
  if (src.isArray(index)) {
    return CommonJson::Ptr(new CommonJsonArray(src.getArray(index)));
  } else if (src.isObject(index)) {
    return CommonJson::Ptr(new CommonJsonObject(src.getObject(index)));
  } else {
    return CommonJson::Ptr(new CommonJsonVar(src.get(index)));
  }
}

CommonJson::Ptr CommonJson::getArrayElement(Poco::JSON::Array::Ptr src, int index) {
  return getArrayElement(*(src), index);
}

void CommonJson::invalid() {
  LOG(FATAL) << "Invalid CommonJson operation";
}

void CommonJsonVar::addToArray(Poco::JSON::Array *dst) {
  dst->add(_x);
}

void CommonJsonArray::addToArray(Poco::JSON::Array *dst) {
  dst->add(_x);
}

void CommonJsonObject::addToArray(Poco::JSON::Array *dst) {
  dst->add(_x);
}

void CommonJsonVar::setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName) {
  dst->set(fieldName, _x);
}

void CommonJsonObject::setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName) {
  dst->set(fieldName, _x);
}

void CommonJsonArray::setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName) {
  dst->set(fieldName, _x);
}

void serializeField(CommonJson::Ptr cobj, const std::string &fieldName, const std::string &value) {
  Poco::JSON::Object::Ptr &obj = cobj->toObject()->get();
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


