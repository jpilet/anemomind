/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MDINDSJSON_H_
#define MDINDSJSON_H_

#include <server/common/Json.h>
#include <server/common/MDInds.h>
#include <server/common/Array.h>

namespace sail {
namespace json {

template <int N>
Poco::JSON::Array serialize(const MDInds<N> &inds) {
  return serializeArray(Arrayi(N, inds.getData()));
}

template <int N>
void deserialize(Poco::JSON::Array src, MDInds<N> *dst) {
  Arrayi dstdata;
  deserializeArray(src, &dstdata);
  assert(dstdata.size() == N);
  *dst = MDInds<N>(dstdata.ptr());
}

}
}

#endif /* MDINDSJSON_H_ */
