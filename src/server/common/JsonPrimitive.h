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

template <typename T>
Poco::Dynamic::Var serialize(T x) {return serializePrimitive(x);}

template <typename T>
bool deserialize(Poco::Dynamic::Var src, T *dst) {return deserializePrimitive(src, dst);}


#undef DEFINE_JSON_PRIMITIVE_INT
#undef DEFINE_JSON_PRIMITIVE

}
} /* namespace sail */

#endif /* JSONPRIMITIVE_H_ */
