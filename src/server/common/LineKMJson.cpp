/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LineKMJson.h"
#include <server/common/LineKM.h>

namespace sail {
namespace json {

Poco::JSON::Object::Ptr serialize(const LineKM &x) {
  Poco::JSON::Object::Ptr dst(new Poco::JSON::Object());
  dst->set("k", x.getK());
  dst->set("m", x.getM());
  return dst;
}

void deserialize(Poco::JSON::Object::Ptr src, LineKM *dst) {
  *dst = LineKM(src->getValue<double>("k"), src->getValue<double>("m"));
}


}
} /* namespace sail */
