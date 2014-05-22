/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Json interface for Physical quantities.
 *  Provides readField() and writeField() for most Quantity<Type> combinations.
 */

#ifndef PHYSICALQUANTITYJSON_H_
#define PHYSICALQUANTITYJSON_H_

#include <Poco/JSON/Object.h>
#include <server/common/PhysicalQuantity.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

namespace {

template <class Quantity, typename Value>
struct JsonQuantityTraits {
};

template <typename Value>
struct JsonQuantityTraits<Duration<Value>, Value> {
    static double serialize(const Duration<Value>& duration) { return duration.seconds(); }
    static Duration<Value> deserialize(double v) { return Duration<Value>::seconds(v); }
    static const char* suffix() { return  "_s"; };
    static const char* quantityName() {return "Duration";}
};

template <typename Value>
struct JsonQuantityTraits<Velocity<Value>, Value> {
    static double serialize(const Velocity<Value>& a) { return a.metersPerSecond(); }
    static Velocity<Value> deserialize(double v) { return Velocity<Value>::metersPerSecond(v); }
    static const char* suffix() { return "_mps"; };
    static const char* quantityName() {return "Velocity";}
};

template <typename Value>
struct JsonQuantityTraits<Angle<Value>, Value> {
    static double serialize(const Angle<Value>& a) { return a.radians(); }
    static Angle<Value> deserialize(double v) { return Angle<Value>::radians(v); }
    static const char* suffix() { return "_rad"; };
    static const char* quantityName() {return "Angle";}
};

template <typename Value>
struct JsonQuantityTraits<Length<Value>, Value> {
    static double serialize(const Length<Value>& a) { return a.meters(); }
    static Length<Value> deserialize(double v) { return Length<Value>::meters(v); }
    static const char* suffix() { return "_m"; };
    static const char* quantityName() {return "Length";}
};

}  // namespace

template<class Quantity>
bool deserializeField(CommonJson::Ptr cobj, std::string fieldPrefix,
               Quantity *out) {
  Poco::JSON::Object::Ptr obj = cobj->toObject();
    std::string fname = fieldPrefix + JsonQuantityTraits<Quantity, typename Quantity::ValueType>::suffix();
    bool is = obj->has(fname);
    if (is) {
        *out = JsonQuantityTraits<Quantity, typename Quantity::ValueType>::deserialize(
            obj->getValue<double>(fname));
        return true;
    }
    *out = Quantity();
    return false;
}

template<class Quantity>
void serializeField(Poco::JSON::Object::Ptr obj, std::string fieldPrefix,
                const Quantity &x) {
  double val =
      JsonQuantityTraits<Quantity, typename Quantity::ValueType>::serialize(x);
  if (!std::isnan(val)) {
    obj->set(
        fieldPrefix +
        JsonQuantityTraits<Quantity, typename Quantity::ValueType>::suffix(),
        val);
  }
}

template <class Quantity>
Poco::JSON::Object::Ptr serialize(const Quantity &x) {
  return toJsonObjectWithField<Quantity>(std::string(x.quantityName()) + x.suffix(),
      x);
}

template <class Quantity>
bool deserialize(Poco::JSON::Object::Ptr src, Quantity *x) {
  return deserializeField(src, std::string(Quantity::quantityName()) + Quantity::suffix(), *x);
}

template <typename T, int N>
CommonJson::Ptr serialize(const Vectorize<T, N> &x) {
  return serialize(Array<T>(N, x.data()));
}

template <typename T, int N>
bool deserialize(CommonJson::Ptr src, Vectorize<T, N> *x) {
  Array<T> arr;
  deserialize(src, &arr);
  assert(arr.size() == N);
  *x = Vectorize<T, N>(arr.ptr());
  return true;
}


}
} /* namespace sail */

#endif /* PHYSICALQUANTITYJSON_H_ */
