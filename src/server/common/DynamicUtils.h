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
#include <server/common/traits.h>

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

// TO_DYNAMIC_FUNCTION has signature
//
//   SerializationInfo TO_DYNAMIC_FUNCTION(const T& src, Poco::Dynamic::Var* dst);
//
// It can be a template function, or a regular function, possibly overloaded
// for many different types.
#define SPECIALIZE_TO_DYNAMIC(TO_DYNAMIC_FUNCTION) \
  template <typename T> \
  struct ToDynamic<T, decltype(TO_DYNAMIC_FUNCTION( \
      std::declval<const T&>(), std::declval<Poco::Dynamic::Var*>()))> { \
    static SerializationInfo apply( \
        const T& src, Poco::Dynamic::Var* dst) { \
      return TO_DYNAMIC_FUNCTION(src, dst); \
    } \
  };
// FROM_DYNAMIC_FUNCTION has signature
//
// SerializationInfo FROM_DYNAMIC_FUNCTION(const Poco::Dynamic::Var& src, T* dst);
//
// It can be a template function, or a regular function, possibly overloaded
// for many different types.
#define SPECIALIZE_FROM_DYNAMIC(FROM_DYNAMIC_FUNCTION) \
  template <typename T> \
  struct FromDynamic<T, decltype(FROM_DYNAMIC_FUNCTION( \
      std::declval<const Poco::Dynamic::Var&>(), std::declval<T*>()))> { \
    static SerializationInfo apply( \
        const Poco::Dynamic::Var& src, T* dst) { \
      return FROM_DYNAMIC_FUNCTION(src, dst); \
    } \
  };

////////////// When toDynamicObject/fromDynamicObject are overloaded
SPECIALIZE_TO_DYNAMIC(toDynamicObject)
SPECIALIZE_FROM_DYNAMIC(fromDynamicObject);

///////////// When the object is sequential
template <typename T>
struct SequentialToDynamic {
  static SerializationInfo apply(const T& seq, Poco::Dynamic::Var* dst) {
    Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
    typedef typename T::value_type V;
    for (auto x: seq) {
      Poco::Dynamic::Var e;
      auto s = ToDynamic<V>::apply(x, &e);
      arr->add(e);
    }
    *dst = Poco::Dynamic::Var(arr);
    return SerializationInfo(SerializationStatus::Success);
  }
};
template <typename T> // Helper, just so that the macro works
using EnabledSequenceToDynamic =
    typename std::enable_if<IsSequenceLike<T>::value,
    SequentialToDynamic<T>>::type;
SPECIALIZE_TO_DYNAMIC(EnabledSequenceToDynamic<T>::apply);

template <typename T>
struct SequentialFromDynamic {
  static SerializationInfo apply(const Poco::Dynamic::Var& csrc, T* dst) {
    try {
      Poco::JSON::Array::Ptr src = csrc.extract<Poco::JSON::Array::Ptr>();
      typedef typename T::value_type V;
      std::vector<V> tmp;
      int count = src->size();
      tmp.reserve(count);
      for (int i = 0; i < count; i++) {
        V x;
        auto k = FromDynamic<V>::apply(src->get(i), &x);
        if (!k) {
          return k;
        }
        tmp.push_back(x);
      }

      // T must have this kind of constructor.
      *dst = T(tmp.begin(), tmp.end());

      return SerializationStatus::Success;
    } catch (const Poco::Exception& ) {
      return SerializationStatus::Failure;
    }
  }
};

template <typename T>
using EnabledSequenceFromDynamic =
    typename std::enable_if<IsSequenceLike<T>::value,
    SequentialFromDynamic<T>>::type;
SPECIALIZE_FROM_DYNAMIC(EnabledSequenceFromDynamic<T>::apply);

/////////////// When it is a map
template <typename T>
struct MapToDynamic {
  static SerializationInfo apply(const T& src,
      Poco::Dynamic::Var* dst) {
    typedef typename T::mapped_type V;
    Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
    for (const auto& kv: src) {
      Poco::Dynamic::Var x;
      auto s = ToDynamic<V>::apply(kv.second, &x);
      if (!bool(s)) {
        std::cout << "Failed to convert" << std::endl;
        return SerializationStatus::Failure;
      }
      obj->set(kv.first, x);
    }
    *dst = Poco::Dynamic::Var(obj);
    return SerializationStatus::Success;
  }
};
template <typename T>
using EnabledMapToDynamic =
    typename std::enable_if<IsStringMap<T>::value,
    MapToDynamic<T>>::type;
SPECIALIZE_TO_DYNAMIC(EnabledMapToDynamic<T>::apply);

template <typename T>
struct MapFromDynamic {
  static SerializationInfo apply(const Poco::Dynamic::Var& src,
      T* dst) {
    typedef typename T::mapped_type V;
    try {
      auto obj = src.extract<Poco::JSON::Object::Ptr>();
      std::vector<std::string> names;
      obj->getNames(names);
      std::vector<std::pair<std::string, V>> kvPairs;
      kvPairs.reserve(names.size());
      for (const auto& name: names) {
        V x;
        auto s = FromDynamic<V>::apply(obj->get(name), &x);
        if (!s) {
          return s;
        }
        kvPairs.push_back({name, x});
      }

      // It must have this constructor!
      *dst = T(kvPairs.begin(), kvPairs.end());

      return SerializationStatus::Success;
    } catch (const Poco::Exception& ) {
      return SerializationStatus::Failure;
    }
  }
};
template <typename T>
using EnabledMapFromDynamic
    = typename std::enable_if<IsMap<T>::value, MapFromDynamic<T>>::type;
SPECIALIZE_FROM_DYNAMIC(EnabledMapFromDynamic<T>::apply);




/////////// Shared ptr
template <typename T>
SerializationInfo sharedPointerToDynamic(
    const std::shared_ptr<T>& src,
    Poco::Dynamic::Var* dst) {
  if (src) {
    return ToDynamic<T>::apply(*src, dst);
  } else {
    *dst = Poco::Dynamic::Var();
  }
  return SerializationStatus::Success;
}
SPECIALIZE_TO_DYNAMIC(sharedPointerToDynamic);

template <typename T>
SerializationInfo sharedPointerFromDynamic(
    const Poco::Dynamic::Var& src,
    std::shared_ptr<T>* dst) {
  if (src.isEmpty()) {
    *dst = std::shared_ptr<T>();
  } else {
    T x;
    auto s = FromDynamic<T>::apply(src, &x);
    if (!s) {
      return s;
    }
    *dst = std::make_shared<T>(x);
  }
  return SerializationStatus::Success;
}
SPECIALIZE_FROM_DYNAMIC(sharedPointerFromDynamic);








class DynamicField {
public:
  typedef std::shared_ptr<DynamicField> Ptr;
  virtual SerializationInfo readFrom(const Poco::JSON::Object::Ptr& src) = 0;
  virtual SerializationInfo writeTo(Poco::JSON::Object::Ptr dst) = 0;
  virtual ~DynamicField() {}
  void setRequired(bool r) {_required = r;}
protected:
  bool _required = true;
};

// Helper class just to make it more convenient
struct WrappedField {
  DynamicField::Ptr field;

  WrappedField optional() const {
    field->setRequired(false);
    return WrappedField{field};
  }
};

template <typename T, bool>
struct ReadFieldFrom {};

template <typename T>
struct ReadFieldFrom<T, true> {
  static SerializationInfo apply(
      bool req,
      const std::string& key, T* ref,
      Poco::JSON::Object::Ptr src) {
    try {
      if (src->has(key)) {
        auto r = FromDynamic<T>::apply(src->get(key), ref);
        if (!bool(r)) {
          return r;
        }
      } else if (req) {
        std::cerr << "Required field '" << key << "' missing.\n";
        return SerializationStatus::Failure;
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
      bool req,
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
    if (bool(x)) {
      dst->set(_key, d);
    }
    return x;
  }

  SerializationInfo readFrom(
      const Poco::JSON::Object::Ptr& src) override {
    return ReadFieldFrom<T, mut>::apply(_required, _key, _ref, src);
  }
private:
  std::string _key;
  T* _ref = nullptr;
};

template <typename T>
WrappedField field(const std::string& k, T& value) {
  return WrappedField{DynamicField::Ptr(
      new Field<T, !std::is_const<T>::value>(k, &value))};
}

Poco::Dynamic::Var makeDynamicMap(
    std::vector<WrappedField> fields);
SerializationInfo fromDynamicMap(const Poco::Dynamic::Var& src,
    std::vector<WrappedField> fields);

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

SerializationInfo writeDynamicToJson(
    Poco::Dynamic::Var x, std::ostream* dst,
    const JsonSettings& s = JsonSettings());

template <typename T>
SerializationInfo writeJson(const T& src, std::ostream* dst,
    const JsonSettings& s = JsonSettings()) {
  Poco::Dynamic::Var obj;
  auto i = ToDynamic<T>::apply(src, &obj);
  if (!i) {
    return i;
  }
  return writeDynamicToJson(obj, dst, s);
}

Poco::Dynamic::Var readDynamicFromJson(std::istream* src);

template <typename T>
SerializationInfo readJson(std::istream* src, T* dst) {
  auto x = readDynamicFromJson(src);
  if (!x) {
    return SerializationInfo(SerializationStatus::Failure);
  }
  return FromDynamic<T>::apply(x, dst);
}

}

#endif /* SERVER_JSON_DYNAMICUTILS_H_ */
