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
#include <server/common/FieldVisitor.h>

/*
 * GUIDE for providing serialization for any type T:
 *
 * 1. Is the type a class/struct that you implemented? Then provide
 *    serialization for that type by having a method like below:
 *
 *    template <typename V>
 *    void visitFields(V* v) {
 *       v->required = false;
 *       v->visit("field_1", field2); // Optional
 *
 *       v->required = true;
 *       v->visit("field_2", field2); // Required
 *    }
 *
 *    Which when serialized, will result in a JSON object
 *      {"field_1": ..., "field_2": ... }
 *    that can be deserialized again. Note that, because field1 is
 *    not required {"field_2": ...} can also be deserialized.
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

template <typename T>
class It_seems_like_you_did_not_implement_ToDynamic_for;

////////////// Main templates used for SFINAE
template <typename T, typename output = SerializationInfo>
struct ToDynamic : public It_seems_like_you_did_not_implement_ToDynamic_for<T> {
  static SerializationInfo apply(
      const T& src, Poco::Dynamic::Var* dst) {
    return SerializationStatus::Failure;
  }
};

template <typename T>
class It_seems_like_you_did_not_implement_FromDynamic_for;

template <typename T, typename output = SerializationInfo>
struct FromDynamic : public It_seems_like_you_did_not_implement_FromDynamic_for<T> {
  static SerializationInfo apply(
      const Poco::Dynamic::Var& src, T* dst) {
    return SerializationStatus::Failure;
  }
};

#define FOREACH_JSON_PRIMITIVE(OP) \
    OP(double) \
    OP(float) \
    OP(std::string) \
    OP(bool) \
    OP(int32_t) \
    OP(uint32_t) \
    OP(int16_t) \
    OP(uint16_t) \
    OP(uint8_t) \
    OP(int8_t)
// Not supported:
// 64-bit integers. Results in compilation error.

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

class FieldDeserializer : public FieldVisitorBase {
public:
  FieldDeserializer(const Poco::JSON::Object::Ptr& src) : _src(src) {}

  template <typename T>
  void visit(const std::string& key, T& dst) {
    if (!_info) {
      return;
    }
    _info = ReadFieldFrom<T, !std::is_const<T>::value>::apply(
        required, key, &dst, _src);
  }

  SerializationInfo info() const {return _info;}
private:
  Poco::JSON::Object::Ptr _src;
  SerializationInfo _info;
};


class FieldSerializer : public FieldVisitorBase {
public:
  FieldSerializer(const Poco::JSON::Object::Ptr& dst) : _dst(dst) {}

  template <typename T>
  void visit(const std::string& key, T& obj) {
    if (!_info) {
      return;
    }
    Poco::Dynamic::Var d;
    _info = ToDynamic<T>::apply(obj, &d);
    if (bool(_info)) {
      _dst->set(key, d);
    }
  }

  SerializationInfo info() const {return _info;}
private:
  Poco::JSON::Object::Ptr _dst;
  SerializationInfo _info;
};

// TODO: it must be more clear...
// Just passing T here will match too many things...
template <typename T>
struct FieldsToDynamic {
  static SerializationInfo apply(
      const T& src0, Poco::Dynamic::Var* dst) {

    // Some cheating ;-)
    // The reason for this is to avoid code duplication.
    // When we are serializing an object, we are not modifying
    // it so it makes sense to refer to it as a constant object.
    // But that would require a the 'visitFields' method to
    // also be const. But when we want to deserialize an object
    // we need it to be non const, and in that case, we would need
    // a non-const 'visitFields'. To spare ourselves from that,
    // lets just cast it to a non-const object and visit the
    // fields.
    //
    // TODO: Are there any real dangers with this? Undefined behaviour?
    // Or is it just ugly, but harmless?
    T& src = const_cast<T&>(src0);

    Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
    FieldSerializer s(obj);
    src.template visitFields<FieldSerializer>(&s);
    if (s.info()) {
      *dst = Poco::Dynamic::Var(obj);
    }
    return s.info();
  }
};
template <typename T> // Helper, just so that the macro works
using EnabledFieldsToDynamic =
    typename std::enable_if<CanVisitFields<T>::value,
    FieldsToDynamic<T>>::type;
SPECIALIZE_TO_DYNAMIC(EnabledFieldsToDynamic<T>::apply);

template <typename T>
struct FieldsFromDynamic {
  static SerializationInfo apply(
      const Poco::Dynamic::Var& src, T* dst) {
    FieldDeserializer d(src.extract<Poco::JSON::Object::Ptr>());
    dst->template visitFields<FieldDeserializer>(&d);
    return d.info();
  }
};
template <typename T>
using EnabledFieldsFromDynamic =
    typename std::enable_if<CanVisitFields<T>::value,
    FieldsFromDynamic<T>>::type;
SPECIALIZE_FROM_DYNAMIC(EnabledFieldsFromDynamic<T>::apply);

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
