/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SPANJSON_H_
#define SPANJSON_H_

#include <Poco/Dynamic/Var.h>
#include <server/common/JsonObjDeserializer.h>
#include <server/common/Span.h>

namespace sail {
namespace json {

template <typename T>
Poco::Dynamic::Var serialize(const Span<T> &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("minv", serialize(x.minv()));
  obj->set("maxv", serialize(x.maxv()));
  return Poco::Dynamic::Var(obj);
}

template <typename T>
bool deserialize(Poco::Dynamic::Var src, Span<T> *dst) {
  ObjDeserializer deser(src);
  T minv, maxv;
  deser.get("minv", &minv);
  deser.get("maxv", &maxv);
  if (deser.success()) {
    *dst = Span<T>(minv, maxv);
    return true;
  }
  return false;
}


}
}



#endif /* SPANJSON_H_ */
