/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSONPRIMITIVE_H_
#define JSONPRIMITIVE_H_

#include <Poco/Dynamic/Var.h>

namespace sail {
namespace json {

template <typename T>
Poco::Dynamic::Var serialize(T x) {return Poco::Dynamic::Var(x);}

template <typename T>
bool deserialize(Poco::Dynamic::Var obj, T *x) {
  try {
    *x = obj.convert<T>();
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}

}
} /* namespace sail */

#endif /* JSONPRIMITIVE_H_ */
