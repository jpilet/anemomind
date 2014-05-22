/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TimeStampJson.h"
#include <server/common/TimeStamp.h>
#include <server/common/logging.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

namespace {
  const char TimeLabel[] = "ms_1970";

  std::string makeFname(std::string prefix) {
    return prefix + "_" + TimeLabel;
  }
}

bool deserializeField(CommonJson::Ptr cobj, const std::string &prefix, TimeStamp *out) {
  Poco::JSON::Object::Ptr obj = cobj->toObject()->get();
  std::string fname = makeFname(prefix);
  if (obj->has(fname)) {
    *out = TimeStamp::fromMilliSecondsSince1970(obj->getValue<Poco::Int64>(fname));
    return true;
  }
  *out = TimeStamp::makeUndefined();
  return false;
}

void serializeField(Poco::JSON::Object::Ptr obj, const std::string &prefix, const TimeStamp &x) {
  obj->set(makeFname(prefix), static_cast<Poco::Int64>(x.toMilliSecondsSince1970()));
}

CommonJson::Ptr serialize(const TimeStamp &src) {
  //return toJsonObjectWithField<TimeStamp>("time", src);
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object()); // Workaround for the line above which won't compile for some reason.
  obj->set("time", src);
  return CommonJson::Ptr(new CommonJsonObject(obj));
}

bool deserialize(CommonJson::Ptr src, TimeStamp *dst) {
  return deserializeField(src, "time", dst);
}







}  // namespace json
}  // namespace sail
