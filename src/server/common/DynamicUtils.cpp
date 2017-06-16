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
  std::cout << "SERIALIZE " << s << std::endl;
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


SerializationInfo fromDynamicMap(const Poco::Dynamic::Var& src,
    const std::initializer_list<DynamicField::Ptr>& fields) {
  try {
    auto obj = src.extract<Poco::JSON::Object::Ptr>();
    for (const auto& f: fields) {
      f->readFrom(obj);
    }
    return SerializationStatus::Success;
  } catch (const std::exception& e) {
    return SerializationStatus::Failure;
  }
}

Poco::Dynamic::Var makeDynamicMap(
    std::vector<DynamicField::Ptr> fields) {

  //return Poco::Dynamic::Var(
  //    Poco::JSON::Object::Ptr(new Poco::JSON::Object()));

  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  for (const auto& f: fields) {
    auto s = f->writeTo(obj);
    if (!bool(s)) {
      std::cout << "FAILED!" << std::endl;
      return Poco::Dynamic::Var();
    }
  }
  auto v = Poco::Dynamic::Var(obj);
  std::cout << "Value of the map : " << v.toString() << std::endl;
  return v;
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


