/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LineKMJson.h"
#include <server/common/LineKM.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

CommonJson::Ptr serialize(const LineKM &x) {
  CommonJson::Ptr obj = CommonJsonObject::make();
  obj->toObject()->set("k", serialize(x.getK()));
  obj->toObject()->set("m", serialize(x.getM()));
  return obj;
}
void deserialize(CommonJson::Ptr src, LineKM *dst) {
  double k = 0, m = 0;
  deserialize(src->toObject()->get("k"), &k);
  deserialize(src->toObject()->get("m"), &m);
  *dst = LineKM(k, m);
}



}
} /* namespace sail */
