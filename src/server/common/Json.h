/*
 *  Created on: 2014-04-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSON_H_
#define JSON_H_

#include <server/common/Array.h>

namespace sail {
namespace json {

// If serializeField, deserializeField are already defined for type T,
// use this templates to build a serialize function.
template <typename T>
Poco::JSON::Object::Ptr toJsonObjectWithField(std::string typeName, const T &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  serializeField(obj, typeName, x);
  return obj;
}

template <typename T>
Poco::JSON::Array serializeArray(Array<T> src) {
  Poco::JSON::Array arr;
  int count = src.size();
  for (int i = 0; i < count; i++) {
    arr.add(serialize(src[i]));
  }
  return arr;
}

template <typename T>
void deserializeArray(Poco::JSON::Array src, Array<T> *dst) {
  int count = src.size();
  *dst = Array<T>(count);
  for (int i = 0; i < count; i++) {
    deserialize(src.getObject(i), dst->ptr(i));
  }
}

}
}



#endif /* JSON_H_ */
