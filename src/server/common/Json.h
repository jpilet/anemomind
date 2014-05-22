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

  // Please first check with isX() before any of these methods is called
  // if you want to avoid an error.
  virtual CommonJsonVar *toVar() {invalid(); return nullptr;}
  virtual CommonJsonArray *toArray() {invalid(); return nullptr;}
  virtual CommonJsonObject *toObject() {invalid(); return nullptr;}



  virtual void addToArray(Poco::JSON::Array *dst) = 0;

  virtual void setObjectField(Poco::JSON::Object::Ptr dst,
      std::string fieldName) = 0;

  static CommonJson::Ptr getObjectField(Poco::JSON::Object::Ptr src,
      std::string fieldName);

  static CommonJson::Ptr getArrayElement(Poco::JSON::Array &src, int index);
  static CommonJson::Ptr getArrayElement(Poco::JSON::Array::Ptr src, int index);

  virtual ~CommonJson() {}
 private:
  void invalid();
};

class CommonJsonVar : public CommonJson {
 public:
  CommonJsonVar(Poco::Dynamic::Var x) : _x(x) {}
  Poco::Dynamic::Var &get() {return _x;}
  bool isDynamicVar() {return true;}
  CommonJsonVar *toVar() {return this;}
  void addToArray(Poco::JSON::Array *dst);
  void setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName);
 private:
  Poco::Dynamic::Var _x;
};

class CommonJsonArray : public CommonJson {
 public:
  CommonJsonArray(Poco::JSON::Array x) : _x(new Poco::JSON::Array(x)) {}
  CommonJsonArray(Poco::JSON::Array::Ptr x) : _x(x) {}

  Poco::JSON::Array::Ptr &get() {return _x;}
  bool isArray() {return true;}
  CommonJsonArray *toArray() {return this;}
  void addToArray(Poco::JSON::Array *dst);
  void setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName);
 private:
  // TODO: Unsure of what is best: Poco::JSON::Array::Ptr or simply Poco::JSON::Array?
  Poco::JSON::Array::Ptr _x;
};

class CommonJsonObject : public CommonJson {
 public:
  CommonJsonObject(Poco::JSON::Object::Ptr x) : _x(x) {}
  Poco::JSON::Object::Ptr &get() {return _x;}
  bool isObject() {return true;}
  CommonJsonObject *toObject() {return this;}
  void addToArray(Poco::JSON::Array *dst);
  void setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName);
 private:
  Poco::JSON::Object::Ptr _x;
};


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
