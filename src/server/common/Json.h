/*
 *  Created on: 2014-04-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSON_H_
#define JSON_H_

#include <server/common/Array.h>
#include <server/common/CommonJson.h>
#include <server/common/string.h>

namespace sail {
namespace json {





//template <typename T>
//class JsonPrimitive {
// public:
//  static const bool IsPrimitive = false;
//  static void readArrayElement(const Poco::JSON::Array &src, int index, T *dst) {}
//};
//
//#define DECLARE_JSON_PRIMITIVE(type) \
//    inline Poco::Dynamic::Var serialize(type x) {return Poco::Dynamic::Var(x);} \
//    template <> \
//    class JsonPrimitive<type> { \
//     public: \
//      static const bool IsPrimitive = true; \
//      static void readArrayElement(const Poco::JSON::Array &src, int index, type *dst) { \
//        *dst = src.getElement<type>(index); \
//      } \
//    }
//DECLARE_JSON_PRIMITIVE(int);
//DECLARE_JSON_PRIMITIVE(double);
//#undef DECLARE_JSON_PRIMITIVE

// If serializeField, deserializeField are already defined for type T,
// use this templates to build a serialize function.
template <typename T>
CommonJson::Ptr toJsonObjectWithField(const std::string &typeName, const T &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  serializeField(obj, typeName, x);
  return CommonJson::Ptr(new CommonJsonObject(obj));
}

template <typename T>
CommonJson::Ptr serializeArray(Array<T> src) {
  Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
  int count = src.size();
  for (int i = 0; i < count; i++) {
    serialize(src[i])->addToArray(arr.get());
  }
  return CommonJson::Ptr(new CommonJsonArray(arr));
}

template <typename T>
CommonJson::Ptr serialize(Array<T> src) {
  return serializeArray(src);
}

template <typename T>
void deserialize(CommonJson::Ptr csrc, Array<T> *dst) {
  assert(csrc->isArray());
  Poco::JSON::Array::Ptr src = csrc->toArray()->get();
  int count = src->size();
  *dst = Array<T>(count);
  for (int i = 0; i < count; i++) {
    deserialize(CommonJson::getArrayElement(src, i), dst->ptr(i));
  }
}

// string
void serializeField(Poco::JSON::Object::Ptr obj, const std::string &fieldName, const std::string &value);
void deserializeField(CommonJson::Ptr obj, const std::string &fieldName, std::string *valueOut);

}
}



#endif /* JSON_H_ */
