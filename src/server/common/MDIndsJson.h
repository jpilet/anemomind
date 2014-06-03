/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MDINDSJSON_H_
#define MDINDSJSON_H_

#include <server/common/MDInds.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

template <int N>
Poco::Dynamic::Var serialize(MDInds<N> inds) {
  return serialize(Arrayi(N, inds.getData()));
}

template <int N>
bool deserialize(Poco::Dynamic::Var obj, MDInds<N> *dst) {
  Arrayi inds;
  if (!deserialize(obj, &inds)) {
    return false;
  }
  if (!(inds.size() == N)) {
    return false;
  }
  *dst = MDInds<N>(inds.ptr());
  return true;
}

}
}

#endif /* MDINDSJSON_H_ */
