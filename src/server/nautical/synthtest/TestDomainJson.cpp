/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TestDomain.h"
#include "TestDomainJson.h"
#include <server/common/TimeStampJson.h>
#include <server/common/PhysicalQuantityJson.h>
#include <server/nautical/GeographicReferenceJson.h>
#include <server/common/Json.h>
#include <server/common/SpanJson.h>
#include <server/common/JsonObjDeserializer.h>


namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const TestSpaceDomain &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("geoRef", serialize(src.geoRef()));
  obj->set("xSpan", serialize(src.xSpan()));
  obj->set("ySpan", serialize(src.ySpan()));
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, TestSpaceDomain *dst) {
  ObjDeserializer deser(src);
  GeographicReference geoRef;
  LengthSpan xSpan, ySpan;
  deser.get("geoRef", &geoRef);
  deser.get("xSpan", &xSpan);
  deser.get("ySpan", &ySpan);
  if (deser.success()) {
    *dst = TestSpaceDomain(geoRef, xSpan, ySpan);
    return true;
  }
  return false;
}

Poco::Dynamic::Var serialize(const TestTimeDomain &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("timeRef", serialize(src.timeRef()));
  obj->set("timeSpan", serialize(src.timeSpan()));
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, TestTimeDomain *dst) {
  ObjDeserializer deser(src);
  TimeStamp timeRef;
  TimeSpan timeSpan;
  deser.get("timeRef", &timeRef);
  deser.get("timeSpan", &timeSpan);
  if (deser.success()) {
    *dst = TestTimeDomain(timeRef, timeSpan);
    return true;
  }
  return false;
}

Poco::Dynamic::Var serialize(const TestDomain &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("time", serialize(src.time()));
  obj->set("space", serialize(src.space()));
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var src, TestDomain *dst) {
  ObjDeserializer deser(src);
  TestSpaceDomain space;
  TestTimeDomain time;
  deser.get("space", &space);
  deser.get("time", &time);
  if (deser.success()) {
    *dst = TestDomain(space, time);
    return true;
  }
  return false;
}



}
} /* namespace sail */
