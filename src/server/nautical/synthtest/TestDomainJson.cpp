/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TestDomainJson.h"
#include <server/nautical/GeographicReferenceJson.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const TestSpaceDomain &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("geoRef", serialize(src.geoRef()));
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, TestSpaceDomain *dst) {

}

Poco::Dynamic::Var serialize(const TestTimeDomain &src);
bool deserialize(Poco::Dynamic::Var src, TestTimeDomain *dst);

Poco::Dynamic::Var serialize(const TestDomain &src);
bool deserialize(Poco::Dynamic::Var src, TestDomain *dst);



}
} /* namespace sail */
