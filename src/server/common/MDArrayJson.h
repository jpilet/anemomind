/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MDARRAYJSON_H_
#define MDARRAYJSON_H_

#include <server/common/MDIndsJson.h>

namespace sail {
namespace json {

template <typename T, int N>
Poco::Dynamic::Var serialize(MDArray<T, N> src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("size", serialize(src.size()));
  obj->set("data", serialize(src.continuousData()));
  return obj;
}

template <typename T, int N>
bool deserialize(Poco::Dynamic::Var csrc, MDArray<T, N> *dst) {
  try {
      Poco::JSON::Object::Ptr src = csrc.extract<Poco::JSON::Object::Ptr>();
      *dst = MDArray<T, N>();
      if (!src->has("size")) {
        return false;
      }
      if (!src->has("data")) {
        return false;
      }
      MDInds<N> size;
      if (!deserialize(src->get("size"), &size)) {
        return false;
      }
      Array<T> data;
      if (!deserialize(src->get("data"), &data)) {
        return false;
      }
      *dst = MDArray<T, N>(size, data);
      return true;
    } catch (Poco::Exception &e) {
      return false;
    }
}

}
}



#endif /* MDARRAYJSON_H_ */
