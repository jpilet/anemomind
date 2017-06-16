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

/*
 * GUIDE for providing serialization for any type T:
 *
 * 1. Is the type a class/struct that you implemented? Then provide
 *    serialization for that type using
 *    DYNAMIC_INTERFACE/DYNAMIC_IMPLEMENTATION
 *    Should probably work also in the case of a template type.
 *
 * 2. Otherwise, if the above is not possible,
 *    is it a non-template type? In that case,
 *    implement fromDynamicObject/toDynamicObject for that type.
 *
 * 3. Otherwise, specialize the FromDynamic/ToDynamic template types.
 *    That is the case for template types that we did not implement,
 *    such as std::vector, etc.
 *
 */


namespace sail {

enum class SerializationStatus {
  Success,
  Failure
};

struct SerializationInfo {
  SerializationStatus status = SerializationStatus::Success;

  SerializationInfo() {}
  SerializationInfo(const SerializationStatus& s) : status(s) {}

  operator bool() const {
    return status != SerializationStatus::Failure;
  }
};

SerializationInfo merge(
    const SerializationInfo& a,
    const SerializationInfo& b);

////////////// Main templates used for SFINAE
template <typename T, typename output = SerializationInfo>
struct ToDynamic {
  static SerializationInfo apply(
      const T& src, Poco::Dynamic::Var* dst) {
    // If you get an error here, it probably means
    // that you did not specialize ToDynamic for type T.
    *dst = src.toDynamic();
    return SerializationInfo();
  }
};

template <typename T, typename output = SerializationInfo>
struct FromDynamic {
  static SerializationInfo apply(
      const Poco::Dynamic::Var& src, T* dst) {
    // If you get an error here, it probably means
    // that you did not specialize FromDynamic for type T.
    return dst->fromDynamic(src);
  }
};

#define FOREACH_JSON_PRIMITIVE(OP) \
    OP(double) \
    OP(float) \
    OP(std::string) \
    OP(bool) \
    OP(int32_t) \
    OP(uint32_t)


//OP(uint16_t) \
//OP(uint8_t) \
//OP(int16_t) \
//
//OP(std::string)*/


#define PRIMITIVE_OPS(T) \
    SerializationInfo toDynamicObject(const T& x, Poco::Dynamic::Var* dst); \
    SerializationInfo fromDynamicObject(const Poco::Dynamic::Var& src, T *x);
FOREACH_JSON_PRIMITIVE(PRIMITIVE_OPS)
#undef PRIMITIVE_OPS

////////////// When toDynamicObject/fromDynamicObject are overloaded
template <typename T>
struct ToDynamic<T, decltype(toDynamicObject(
    std::declval<const T&>(), std::declval<Poco::Dynamic::Var*>()))> {
  static SerializationInfo apply(
      const T& src, Poco::Dynamic::Var* dst) {
    return toDynamicObject(src, dst);
  }
};
template <typename T>
struct FromDynamic<T, decltype(fromDynamicObject(
    std::declval<const Poco::Dynamic::Var&>(), std::declval<T*>()))> {
  static SerializationInfo apply(
      const Poco::Dynamic::Var& src, T* dst) {
    return fromDynamicObject(src, dst);
  }
};

class DynamicField {
public:
  typedef std::shared_ptr<DynamicField> Ptr;
  virtual SerializationInfo readFrom(const Poco::JSON::Object::Ptr& src) = 0;
  virtual SerializationInfo writeTo(Poco::JSON::Object::Ptr dst) = 0;
  virtual ~DynamicField() {}
};

template <typename T, bool>
struct ReadFieldFrom {};

template <typename T>
struct ReadFieldFrom<T, true> {
  static SerializationInfo apply(
      const std::string& key, T* ref,
      Poco::JSON::Object::Ptr src) {
    try {
      auto r = FromDynamic<T>::apply(src->get(key), ref);
      if (!bool(r)) {
        return r;
      }
    } catch (const std::exception& e) {
      return SerializationStatus::Failure;
    }
    return SerializationStatus::Success;
  }
};

template <typename T>
struct ReadFieldFrom<T, false> {
  static SerializationInfo apply(
      const std::string&, T*, Poco::JSON::Object::Ptr) {
    // Operation not supported: We cannot change constant data.
    return SerializationStatus::Failure;
  }
};



template <typename T, bool mut>
class Field : public DynamicField {
public:
  Field(const std::string& key, T* ref) :
    _key(key), _ref(ref) {}

  SerializationInfo writeTo(Poco::JSON::Object::Ptr dst) override {
    Poco::Dynamic::Var d;
    auto x = ToDynamic<T>::apply(*_ref, &d);
    std::cout << "Value of d = " << d.toString() << std::endl;
    if (bool(x)) {
      std::cout << "WRITE " << _key << std::endl;
      dst->set(_key, d);
    }
    return x;
  }

  SerializationInfo readFrom(
      const Poco::JSON::Object::Ptr& src) override {
    return ReadFieldFrom<T, mut>::apply(_key, _ref, src);
  }
private:
  std::string _key;
  T* _ref = nullptr;
};

template <typename T>
DynamicField::Ptr field(const std::string& k, T& value) {
  return DynamicField::Ptr(
      new Field<T, !std::is_const<T>::value>(k, &value));
}

Poco::Dynamic::Var makeDynamicMap(
    std::vector<DynamicField::Ptr> fields);
SerializationInfo fromDynamicMap(const Poco::Dynamic::Var& src,
    std::vector<DynamicField::Ptr> fields);

//#define TO_DYNAMIC_CONVERTER(type, converter) \
//  struct ToDn

#define DYNAMIC_INTERFACE \
    Poco::Dynamic::Var toDynamic() const;
    //SerializationInfo fromDynamic(const Poco::Dynamic::Var& src);

#define DYNAMIC_IMPLEMENTATION(ClassName, ...) \
    Poco::Dynamic::Var ClassName::toDynamic() const { \
      /*return Poco::Dynamic::Var(Poco::JSON::Object::Ptr(new Poco::JSON::Object()));*/ \
      return makeDynamicMap({__VA_ARGS__}); \
    }/* \
    SerializationInfo ClassName::fromDynamic(const Poco::Dynamic::Var& src) { \
      return fromDynamicMap(src, {__VA_ARGS__}); \
    }*/


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
