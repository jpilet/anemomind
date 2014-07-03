/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TimeStampJson.h"
#include <server/common/TimeStamp.h>
#include <server/common/logging.h>
#include <server/common/Json.h>

#include <server/common/Json.impl.h>

namespace sail {
namespace json {

namespace {
  const char TimeLabel[] = "ms_1970";

  std::string makeFname(std::string prefix) {
    return prefix + "_" + TimeLabel;
  }
}

bool deserializeField(Poco::Dynamic::Var cobj, const std::string &prefix, TimeStamp *out) {
  try {
    Poco::JSON::Object::Ptr obj = cobj.extract<Poco::JSON::Object::Ptr>();
    std::string fname = makeFname(prefix);
    if (obj->has(fname)) {
      *out = TimeStamp::fromMilliSecondsSince1970(obj->getValue<Poco::Int64>(fname));
      return true;
    }
    *out = TimeStamp::makeUndefined();
    return false;
  } catch (Poco::Exception &e) {
    return false;
  }
}

void serializeField(Poco::JSON::Object::Ptr obj, const std::string &prefix, const TimeStamp &x) {
  obj->set(makeFname(prefix), static_cast<Poco::Int64>(x.toMilliSecondsSince1970()));
}

Poco::Dynamic::Var serialize(const TimeStamp &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  serializeField(obj, "time", src);
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, TimeStamp *dst) {
  return deserializeField(src, "time", dst);
}







}  // namespace json
}  // namespace sail
