/*
 *  Created on: May 22, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CommonJson.h"
#include <server/common/logging.h>

namespace sail {
namespace json {

CommonJson::Ptr CommonJson::getOtherObjectField(Poco::JSON::Object::Ptr src,
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

CommonJson::Ptr CommonJson::getOtherArrayElement(Poco::JSON::Array &src, int index) {
  if (src.isArray(index)) {
    return CommonJson::Ptr(new CommonJsonArray(src.getArray(index)));
  } else if (src.isObject(index)) {
    return CommonJson::Ptr(new CommonJsonObject(src.getObject(index)));
  } else {
    return CommonJson::Ptr(new CommonJsonVar(src.get(index)));
  }
}

CommonJson::Ptr CommonJson::getOtherArrayElement(Poco::JSON::Array::Ptr src, int index) {
  return getOtherArrayElement(*(src), index);
}

void CommonJson::invalid() const {
  LOG(FATAL) << "Invalid CommonJson operation";
}

void CommonJsonVar::addToOtherArray(Poco::JSON::Array *dst) {
  dst->add(_x);
}

void CommonJsonArray::addToOtherArray(Poco::JSON::Array *dst) {
  dst->add(_x);
}

CommonJson::Ptr CommonJsonObject::make() {
  return CommonJson::Ptr(new CommonJsonObject());
}

CommonJsonObject::CommonJsonObject() : _x(new Poco::JSON::Object()) {}

void CommonJsonObject::addToOtherArray(Poco::JSON::Array *dst) {
  dst->add(_x);
}

void CommonJsonVar::setOtherObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName) {
  dst->set(fieldName, _x);
}

void CommonJsonObject::setOtherObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName) {
  dst->set(fieldName, _x);
}

void CommonJsonArray::setOtherObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName) {
  dst->set(fieldName, _x);
}

CommonJsonArray::CommonJsonArray() {
  _x = Poco::JSON::Array::Ptr(new Poco::JSON::Array());
}

CommonJson::Ptr CommonJsonArray::make() {
  return CommonJson::Ptr(new CommonJsonArray());
}

void CommonJsonVar::stringify(std::ostream& out, unsigned int indent , int step ) const {
  LOG(FATAL) << "Cannot stringify a var";
}

void CommonJsonArray::stringify(std::ostream& out, unsigned int indent , int step ) const {
  _x->stringify(out, indent, step);
}

CommonJson::Ptr CommonJsonArray::get(int index) const {
  return CommonJson::getOtherArrayElement(_x, index);
}

void CommonJsonArray::add(CommonJson::Ptr obj) {
  obj->addToOtherArray(_x.get());
}


void CommonJsonObject::stringify(std::ostream& out, unsigned int indent , int step ) const {
  _x->stringify(out, indent, step);
}

CommonJson::Ptr CommonJsonObject::get(const std::string &key) const {
  return CommonJson::getOtherObjectField(_x, key);
}

void CommonJsonObject::set(const std::string &key, CommonJson::Ptr obj) {
  obj->setOtherObjectField(_x, key);
}



}
} /* namespace sail */
