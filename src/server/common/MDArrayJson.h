/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MDARRAYJSON_H_
#define MDARRAYJSON_H_

#include <server/common/MDIndsJson.h>
#include <server/common/JsonObjDeserializer.h>

namespace sail {
namespace json {

template <typename T, int N>
Poco::Dynamic::Var serialize(MDArray<T, N> src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object::Ptr());
  obj->set("size", serialize(src.size()));
  obj->set("data", serialize(src.continuousData()));
  return obj;
}

template <typename T, int N>
bool deserialize(Poco::Dynamic::Var src, MDArray<T, N> *dst) {
  MDInds<N> size;
  Array<T> data;
  ObjDeserializer deser(src);
  deser.get("size", &size);
  deser.get("data", &data);
  if (deser.success()) {
    *dst = MDArray<T, N>(size, data);
    return true;
  } else {
    *dst = MDArray<T, N>();
    return false;
  }
}

}
}



#endif /* MDARRAYJSON_H_ */
