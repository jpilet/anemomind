/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TimeStampJson.h"
#include <server/common/TimeStamp.h>

namespace sail {
namespace json {

namespace {
  std::string makeFname(std::string prefix) {
    return prefix + "-milliseconds-since-1970";
  }
}

bool deserializeField(Poco::JSON::Object::Ptr obj, std::string prefix, TimeStamp *out) {
  std::string fname = makeFname(prefix);
  if (obj->has(fname)) {
    *out = TimeStamp::fromMilliSecondsSince1970(obj->getValue<Poco::Int64>(fname));
    return true;
  }
  *out = TimeStamp::makeUndefined();
  return false;
}

void serializeField(Poco::JSON::Object::Ptr obj, std::string prefix, const TimeStamp &x) {
  obj->set(makeFname(prefix), static_cast<Poco::Int64>(x.toMilliSecondsSince1970()));
}

}  // namespace json
}  // namespace sail
