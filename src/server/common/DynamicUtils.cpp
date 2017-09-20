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
SerializationInfo serializePrimitive(const char* s, T x, Poco::Dynamic::Var* dst) {
  *dst = Poco::Dynamic::Var(x);
  return SerializationInfo();
}

template <typename T>
SerializationInfo deserializePrimitive(const char* s, Poco::Dynamic::Var obj, T *x) {
  try {
    *x = obj.convert<T>();
    return SerializationInfo();
  } catch (Poco::Exception &e) {
    return SerializationInfo(SerializationStatus::Failure);
  }
}

#define PRIMITIVE_OPS(T) \
    SerializationInfo toDynamicObject(const T& x, Poco::Dynamic::Var* dst) {return serializePrimitive<T>(#T, x, dst);} \
    SerializationInfo fromDynamicObject(const Poco::Dynamic::Var& src, T *x) {return deserializePrimitive<T>(#T, src, x);}
FOREACH_JSON_PRIMITIVE(PRIMITIVE_OPS)
#undef PRIMITIVE_OPS

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

SerializationInfo writeDynamicToJson(
    Poco::Dynamic::Var x, std::ostream* file,
    const JsonSettings& s) {
  try {
    Poco::JSON::Stringifier::stringify(
        x, *file, s.indent, s.step, s.preserveInsertionOrder);
    return SerializationInfo();
  } catch (const Poco::Exception& e) {
    return SerializationInfo(SerializationStatus::Failure);
  }

}

Poco::Dynamic::Var readDynamicFromJson(std::istream* src) {
  Poco::JSON::Parser parser;
  Poco::SharedPtr<Poco::JSON::ParseHandler> handler(new Poco::JSON::ParseHandler());
  parser.setHandler(handler);
  try {
    parser.parse(*src);
    return handler->asVar();
  } catch (Poco::Exception &e) {
    return Poco::Dynamic::Var();
  }
}

}


