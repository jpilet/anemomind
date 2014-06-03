/*
 *  Created on: 2014-04-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSON_H_
#define JSON_H_

#include <Poco/JSON/Object.h>
#include <server/common/Array.h>
#include <server/common/string.h>
#include <server/common/logging.h>

namespace sail {
namespace json {

template <typename T>
Poco::Dynamic::Var serialize(T x) {return Poco::Dynamic::Var(x);}

template <typename T>
bool deserialize(Poco::Dynamic::Var obj, T *x) {
  try {
    *x = obj.convert<T>();
    return true;
  } catch (Poco::Exception &e) {
    LOG(INFO) << std::string("Failed to deserialize primitive: ") + e.message();
    return false;
  }
}



// If serializeField, deserializeField are already defined for type T,
// use this templates to build a serialize function.
template <typename T>
Poco::Dynamic::Var toJsonObjectWithField(const std::string &typeName, const T &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  serializeField(obj, typeName, x);
  return Poco::Dynamic::Var(obj);
}

template <typename T>
Poco::Dynamic::Var serializeArray(Array<T> src) {
  Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
  int count = src.size();
  for (int i = 0; i < count; i++) {
    arr->add(serialize(src[i]));
  }
  return Poco::Dynamic::Var(arr);
}

template <typename T>
Poco::Dynamic::Var serialize(Array<T> src, std::function<Poco::Dynamic::Var(T)> customSerializer) {
  Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
  int count = src.size();
  for (int i = 0; i < count; i++) {
    customSerializer(src[i])->addToOtherArray(arr.get());
  }
  return Poco::Dynamic::Var(arr);
}

template <typename T>
Poco::Dynamic::Var serialize(Array<T> src) {
  return serializeArray(src);
}

template <typename T>
void deserialize(Poco::Dynamic::Var csrc, Array<T> *dst) {
  assert(csrc.type() == typeid(Poco::JSON::Array::Ptr));
  Poco::JSON::Array::Ptr src = csrc.extract<Poco::JSON::Array::Ptr>();
  int count = src->size();
  *dst = Array<T>(count);
  for (int i = 0; i < count; i++) {
    deserialize(src->get(i), dst->ptr(i));
  }
}

// string
void serializeField(Poco::JSON::Object::Ptr obj, const std::string &fieldName, const std::string &value);
void deserializeField(Poco::Dynamic::Var obj, const std::string &fieldName, std::string *valueOut);

void stringify(Poco::Dynamic::Var x, std::ostream *out, unsigned int indent = 0, int step = -1);

}
}



#endif /* JSON_H_ */
