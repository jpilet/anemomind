/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSONPRIMITIVE_H_
#define JSONPRIMITIVE_H_

#include <Poco/Dynamic/Var.h>
#include <string>

namespace sail {
namespace json {

template <typename T>
Poco::Dynamic::Var serializePrimitive(T x) {return Poco::Dynamic::Var(x);}

template <typename T>
bool deserializePrimitive(Poco::Dynamic::Var obj, T *x) {
  try {
    *x = obj.convert<T>();
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}

//#define EXPLICIT_JSON_PRIMITIVES 1
#ifdef EXPLICIT_JSON_PRIMITIVES
  template <typename T>
  Poco::Dynamic::Var serialize(T x) {return serializePrimitive(x);}

  template <typename T>
  bool deserialize(Poco::Dynamic::Var src, T *dst) {return deserializePrimitive(src, dst);}
#else
  #define DEFINE_JSON_PRIMITIVE(type) \
      inline Poco::Dynamic::Var serialize(type x) {return serializePrimitive(x);} \
      inline bool deserialize(Poco::Dynamic::Var src, type *dst) {return deserializePrimitive(src, dst);}
  #define DEFINE_JSON_INT(type) \
      DEFINE_JSON_PRIMITIVE(unsigned type) \
      DEFINE_JSON_PRIMITIVE(type)

  DEFINE_JSON_INT(char)
  DEFINE_JSON_INT(short)
  DEFINE_JSON_INT(int)
  DEFINE_JSON_INT(long int)
  DEFINE_JSON_PRIMITIVE(float)
  DEFINE_JSON_PRIMITIVE(double)
  DEFINE_JSON_PRIMITIVE(bool)
  DEFINE_JSON_PRIMITIVE(std::string)

  #undef DEFINE_JSON_INT
  #undef DEFINE_JSON_PRIMITIVE
#endif

}
} /* namespace sail */

#endif /* JSONPRIMITIVE_H_ */
