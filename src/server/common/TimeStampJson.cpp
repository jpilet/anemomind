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
    return prefix + "-int64";
  }
}

void readField(Poco::JSON::Object::Ptr obj, std::string prefix, TimeStamp *out, bool require = true) {
  std::string fname = makeFname(prefix);
  if (obj->has(fname)) {
    *out = TimeStamp::fromInteger(obj->getValue<int64_t>(fname));
  } else if (require) {
    throw MissingFieldException(obj, fname);
  } else {
    *out = TimeStamp::makeUndefined();
  }
}

void writeField(Poco::JSON::Object::Ptr obj, std::string prefix, const TimeStamp &x) {
  obj->set(makeFname(prefix), x.toInteger());
}
}

} /* namespace sail */
