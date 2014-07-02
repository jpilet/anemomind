/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SPANJSON_H_
#define SPANJSON_H_

#include <server/common/Span.h>
#include <Poco/Dynamic/Var.h>

namespace sail {
namespace json {

template <typename T>
Poco::Dynamic::Var serialize(const Span<T> &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("minv", serialize(x.minv()));
  obj->set("maxv", serialize(x.maxv()));
  obj->set("initialized", serialize(x.initialized()));
  return Poco::Dynamic::Var(obj);
}

template <typename T>
bool deserialize(Poco::Dynamic::Var src, Span<T> *dst) {
  try {
    Poco::JSON::Object::Ptr obj = src.extract<Poco::JSON::Object::Ptr>();
    T minv, maxv;
    if (!deserialize(obj->get("minv"), &minv)) {
      return false;
    }
    if (!deserialize(obj->get("maxv"), &maxv)) {
      return false;
    }
    bool initialized;
    if (!deserialize(obj->get("initialized"), &initialized)) {
      return false;
    }
    *dst = Span<T>(minv, maxv);
    return obj;
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}


}
}



#endif /* SPANJSON_H_ */
