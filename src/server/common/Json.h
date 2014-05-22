/*
 *  Created on: 2014-04-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSON_H_
#define JSON_H_

#include <server/common/Array.h>
#include <Poco/JSON/Object.h>

namespace sail {
namespace json {


/*
 * The following classes are intended to simplify
 * writing Json serialization routines.
 *
 * The problem
 * with the Poco json library is that fields in
 * objects and arrays can be either Array, Object or Var.
 * These classes wrap all these objects into a
 * CommonJson object.
 */
class CommonJsonVar;
class CommonJsonArray;
class CommonJsonObject;
class CommonJson {
 public:
  typedef std::shared_ptr<CommonJson> Ptr;
  virtual bool isDynamicVar() const {return false;}
  virtual bool isArray() const {return false;}
  virtual bool isObject() const {return false;}
  virtual CommonJsonVar *toVar() {return nullptr;}
  virtual CommonJsonArray *toArray() {return nullptr;}
  virtual CommonJsonObject *toObject() {return nullptr;}
  virtual ~CommonJson() {}
};

class CommonJsonVar {
 public:
  CommonJsonVar(Poco::Dynamic::Var x) : _x(x) {}
  Poco::Dynamic::Var &get() {return _x;}
  bool isDynamicVar() {return true;}
  CommonJsonVar *toVar() {return this;}
 private:
  Poco::Dynamic::Var _x;
};

class CommonJsonArray {
 public:
  CommonJsonArray(Poco::JSON::Array x) : _x(x) {}
  Poco::Dynamic::Var &get() {return _x;}
  bool isArray() {return true;}
  CommonJsonArray *toArray() {return this;}
 private:
  Poco::Dynamic::Var _x;
};


template <typename T>
class JsonPrimitive {
 public:
  static const bool IsPrimitive = false;
  static void readArrayElement(const Poco::JSON::Array &src, int index, T *dst) {}
};

#define DECLARE_JSON_PRIMITIVE(type) \
    inline Poco::Dynamic::Var serialize(type x) {return Poco::Dynamic::Var(x);} \
    template <> \
    class JsonPrimitive<type> { \
     public: \
      static const bool IsPrimitive = true; \
      static void readArrayElement(const Poco::JSON::Array &src, int index, type *dst) { \
        *dst = src.getElement<type>(index); \
      } \
    }
DECLARE_JSON_PRIMITIVE(int);
DECLARE_JSON_PRIMITIVE(double);
#undef DECLARE_JSON_PRIMITIVE

// If serializeField, deserializeField are already defined for type T,
// use this templates to build a serialize function.
template <typename T>
Poco::JSON::Object::Ptr toJsonObjectWithField(const std::string &typeName, const T &x) {
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
    if (src.isObject(i)) {
      deserialize(src.getObject(i), dst->ptr(i));
    } else {
      JsonPrimitive<T>::readArrayElement(src, i, dst->ptr(i));
    }
  }
}

template <typename T>
void deserializeArray(Poco::JSON::Array::Ptr src, Array<T> *dst) {
  deserializeArray(*src, dst);
}

// string
void serializeField(Poco::JSON::Object::Ptr obj, const std::string &fieldName, const std::string &value);
void deserializeField(Poco::JSON::Object::Ptr obj, const std::string &fieldName, std::string *valueOut);

}
}



#endif /* JSON_H_ */
