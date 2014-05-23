/*
 *  Created on: May 22, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CommonJson.h"
#include <server/common/logging.h>

namespace sail {
namespace json {

CommonJson::Ptr CommonJson::getObjectField(Poco::JSON::Object::Ptr src,
    std::string fieldName) {
  if (!src->has(fieldName)) {
    return CommonJson::Ptr();
  }
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

void CommonJson::invalid() const {
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

void CommonJsonVar::stringify(std::ostream& out, unsigned int indent , int step ) const {
  LOG(FATAL) << "Cannot stringify a var";
}

void CommonJsonArray::stringify(std::ostream& out, unsigned int indent , int step ) const {
  _x->stringify(out, indent, step);
}

void CommonJsonObject::stringify(std::ostream& out, unsigned int indent , int step ) const {
  _x->stringify(out, indent, step);
}


}
} /* namespace sail */
