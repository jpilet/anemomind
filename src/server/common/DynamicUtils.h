/*
 * utils.h
 *
 *  Created on: 15 Jun 2017
 *      Author: jonas
 */

#ifndef SERVER_JSON_DYNAMICUTILS_H_
#define SERVER_JSON_DYNAMICUTILS_H_

#include <Poco/JSON/Object.h>
#include <memory>

namespace sail {

enum class SerializationStatus {
  Success,
  Failure
};

struct SerializationInfo {
  SerializationStatus status = SerializationStatus::Success;
  Poco::Dynamic::Var output;

  SerializationInfo() {}
  SerializationInfo(const SerializationStatus& s) : status(s) {}

  operator bool() const {
    return status == SerializationStatus::Success;
  }
};

////////////// Main templates used for SFINAE
template <typename T, typename output = SerializationInfo>
struct ToDynamic {
  static SerializationInfo apply(
      const T& src, Poco::Dynamic::Var* dst) {
    *dst = src.toDynamic();
    return SerializationInfo();
  }
};

template <typename T, typename output = SerializationInfo>
struct FromDynamic {
  static SerializationInfo apply(
      const Poco::Dynamic::Var& src, T* dst) {
    return dst->fromDynamic(src);
  }
};

#define FOREACH_JSON_PRIMITIVE(OP) \
    OP(double) \
    OP(float) \
    OP(std::string) \
    OP(int32_t) \
    OP(uint32_t) \
    OP(bool)


//OP(uint16_t) \
//OP(uint8_t) \
//OP(int16_t) \
//
//OP(std::string)*/


#define PRIMITIVE_OPS(T) \
    SerializationInfo toDynamic(const T& x, Poco::Dynamic::Var* dst); \
    SerializationInfo fromDynamic(const Poco::Dynamic::Var& src, T *x);
FOREACH_JSON_PRIMITIVE(PRIMITIVE_OPS)
#undef PRIMITIVE_OPS
////////////// When toDynamic/fromDynamic are overloaded
template <typename T>
struct ToDynamic<T, decltype(toDynamic(
    std::declval<T>(), std::declval<Poco::Dynamic::Var*>()))> {
  static SerializationInfo apply(
      const T& src, Poco::Dynamic::Var* dst) {
    return toDynamic(src, dst);
  }
};

class DynamicField {
public:
  typedef std::shared_ptr<DynamicField> Ptr;

  virtual SerializationInfo writeTo(Poco::JSON::Object::Ptr dst) = 0;
  virtual ~DynamicField() {}
};

template <typename T>
class Field : public DynamicField {
public:
  Field(const std::string& key, T& ref) :
    _key(key), _mutableRef(&ref) {}

  SerializationInfo writeTo(Poco::JSON::Object::Ptr dst) override {
    Poco::Dynamic::Var d;
    auto x = ToDynamic<T>::apply(*_mutableRef, &d);
    if (x.status == SerializationStatus::Success) {
      dst->set(_key, x);
    }
    return x;
  }
private:
  std::string _key;
  T* _mutableRef = nullptr;
};

template <typename T>
DynamicField::Ptr field(const std::string& k, T& value) {
  return DynamicField::Ptr(new Field<T>(k, value));
}

Poco::Dynamic::Var makeDynamicMap(
    const std::initializer_list<DynamicField::Ptr>& fields);
SerializationInfo fromDynamicMap(const Poco::Dynamic::Var& src,
    const std::initializer_list<DynamicField::Ptr>& fields);

//#define TO_DYNAMIC_CONVERTER(type, converter) \
//  struct ToDn

#define DYNAMIC_INTERFACE \
    Poco::Dynamic::Var toDynamic() const; \
    SerializationInfo fromDynamic(const Poco::Dynamic::Var& src);

#define DYNAMIC_IMPLEMENTATION(ClassName, ...) \
    Poco::Dynamic::Var ClassName::toDynamic() const { \
      return makeDynamicMap({__VA_ARGS__}); \
    } \
    SerializationInfo ClassName::fromDynamic(const Poco::Dynamic::Var& src) { \
      return fromDynamicMap(src, {__VA_ARGS__}); \
    }


struct JsonSettings {
  int indent = 0;
  int step = 0;
  bool preserveInsertionOrder = false;
};

void outputJson(
    Poco::Dynamic::Var x, std::ostream* dst,
    const JsonSettings& s = JsonSettings());

}

#endif /* SERVER_JSON_DYNAMICUTILS_H_ */
