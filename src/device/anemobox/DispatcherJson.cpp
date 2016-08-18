/*
 * DispatcherJson.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <device/anemobox/DispatcherJson.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/TimestampJson.h>
#include <server/common/PhysicalQuantityJson.h>
#include <server/nautical/GeographicPositionJson.h>
#include <Poco/JSON/Stringifier.h>


namespace sail {
namespace json {

class DispatcherChannelJsonVisitor {
 public:

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
      // TODO: write your code here
  }
};

Poco::Dynamic::Var makeEmptyObject() {
  return Poco::Dynamic::Var(Poco::JSON::Object::Ptr(
      new Poco::JSON::Object()));
}



Poco::Dynamic::Var serialize(const AbsoluteOrientation &x) {
  Poco::JSON::Object::Ptr y(new Poco::JSON::Object());
  y->set("heading", serialize(x.heading));
  y->set("pitch", serialize(x.pitch));
  y->set("roll", serialize(x.roll));
  return Poco::Dynamic::Var(y);
}


// TODO: The other datatypes too.


template <typename T>
Poco::Dynamic::Var serializeTimedValue(const TimedValue<T> &x) {
  Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
  arr->add(serialize(x.time));
  arr->add(serialize(x.value));
  return Poco::Dynamic::Var(arr);
}

template <typename T>
Poco::Dynamic::Var serializeData(const TimedSampleCollection<T> &samples) {
  Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
  for (auto x: samples) {
    arr->add(serializeTimedValue<T>(x));
  }
  return Poco::Dynamic::Var(arr);
}

template <typename T>
Poco::Dynamic::Var serializeForType(std::shared_ptr<DispatchData> d) {
  return serializeData<T>(dynamic_cast<TypedDispatchData<T>*>(d.get())
      ->dispatcher()->values());
}

Poco::Dynamic::Var serialize(DataCode code, std::shared_ptr<DispatchData> d) {
  switch (code) {
#define SERIALIZE_FOR_TYPE(handle, code, shortname, type, description) \
  case handle: return serializeForType<type>(d);
  FOREACH_CHANNEL(SERIALIZE_FOR_TYPE)
#undef SERIALIZE_FOR_TYPE
  default:
    return makeEmptyObject();
  };
  return makeEmptyObject();
}

Poco::Dynamic::Var serialize(
    DataCode code,
    const std::map<std::string, std::shared_ptr<DispatchData> > &m) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
  for (auto kv: m) {
    x->set(kv.first, serialize(code, kv.second));
  }
  return Poco::Dynamic::Var(x);
}

Poco::Dynamic::Var serialize(const Dispatcher *d) {
  Poco::JSON::Object::Ptr x(new Poco::JSON::Object());
  auto m = d->allSources();
  for (auto ch: m) {
    x->set(wordIdentifierForCode(ch.first), serialize(ch.first, ch.second));
  }
  return Poco::Dynamic::Var(x);
}

void outputJson(const Dispatcher *d, std::ostream *dst) {
  Poco::JSON::Stringifier::stringify(
      serialize(d),
      *dst, 0, 0);
}


}
}

