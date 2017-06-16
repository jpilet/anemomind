/*
 * DynamicUtils.cpp
 *
 *  Created on: 15 Jun 2017
 *      Author: jonas
 */

#include <server/common/DynamicUtils.h>
#include <fstream>
#include <Poco/JSON/ParseHandler.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>

namespace sail {

template <typename T>
SerializationInfo serializePrimitive(T x, Poco::Dynamic::Var* dst) {
  *dst = Poco::Dynamic::Var(x);
  return SerializationInfo();
}

template <typename T>
SerializationInfo deserializePrimitive(Poco::Dynamic::Var obj, T *x) {
  try {
    *x = obj.convert<T>();
    return SerializationInfo();
  } catch (Poco::Exception &e) {
    return SerializationInfo(SerializationStatus::Failure);
  }
}

#define PRIMITIVE_OPS(T) \
    SerializationInfo toDynamic(const T& x, Poco::Dynamic::Var* dst) {return serializePrimitive<T>(x, dst);} \
    SerializationInfo fromDynamic(const Poco::Dynamic::Var& src, T *x) {return deserializePrimitive<T>(src, x);}
FOREACH_JSON_PRIMITIVE(PRIMITIVE_OPS)
#undef PRIMITIVE_OPS


Poco::Dynamic::Var makeDynamicMap(
    const std::initializer_list<DynamicField::Ptr>& fields) {
  return Poco::Dynamic::Var();
}

SerializationInfo fromDynamicMap(const Poco::Dynamic::Var& src,
    const std::initializer_list<DynamicField::Ptr>& fields) {
  return SerializationStatus::Success;
}

Poco::Dynamic::Var readJson(const std::string &filename) {
  std::ifstream file(filename);
  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());
  parser.setHandler(handler);
  try {
    parser.parse(file);
    return handler->asVar();
  } catch (Poco::Exception &e) {
    return Poco::Dynamic::Var();
  }
}

void outputJson(
    Poco::Dynamic::Var x, std::ostream* file,
    const JsonSettings& s) {
  Poco::JSON::Stringifier::stringify(
      x, *file, s.indent, s.step, s.preserveInsertionOrder);
}

}

